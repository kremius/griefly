package main

import (
	"fmt"
	"log"
	"math/rand"
	"strconv"
	"strings"
	"time"
)

const (
	PlayerQueueLength   = 1024 * 2
	RegistryQueueLength = 32

	SessionDirTimeFormat = "2006-01-02_15-04-05_07-00"
)

var (
	TickInterval        = 100 * time.Millisecond
	StartHashCheckEvery = 1
	HashCheckTimeout    = 500 * time.Millisecond
	SendConnCountEvery  = 5
)

type newPlayerReply struct {
	id             int
	master         bool
	mapDownloadURL string
	inbox          chan *Envelope
	errReply       *Envelope
}

type PlayerEnvelope struct {
	m        *Envelope
	response chan newPlayerReply
}

type PlayerInfo struct {
	id       int
	userInfo *UserInfo
}

type versionCheck struct {
	version  string
	response chan versionCheckResponse
}

type versionCheckResponse struct {
	ok      bool
	version string
}

type hashCheck struct {
	deadline           time.Time
	waitingClientCount int
	hashes             map[int]int
}

func (hc *hashCheck) add(id int, msg *MessageHash) {
	hc.hashes[id] = *msg.Hash
	hc.waitingClientCount--
}

type playerDrop struct {
	id     int
	reason *Envelope
}

type RegistryConfig struct {
	DumpRoot string
}

type Registry struct {
	nextID  int
	clients map[int]chan *Envelope
	players map[string]PlayerInfo

	masterID      int
	clientVersion string

	newPlayers   chan PlayerEnvelope
	versionCheck chan versionCheck

	droppedPlayers chan playerDrop

	hashes            map[int]*hashCheck
	currentTick       int
	nextTickCallbacks []func(*Registry)

	inbox       chan *Envelope
	ticker      *time.Ticker
	assetServer *AssetServer
	dumper      *DumpWriter
	db          DB
	collector   *StatsCollector

	config *RegistryConfig
}

func newRegistry(as *AssetServer, config *RegistryConfig, db DB, collector *StatsCollector) *Registry {
	return &Registry{1, make(map[int]chan *Envelope), make(map[string]PlayerInfo),
		-1, "", make(chan PlayerEnvelope), make(chan versionCheck), make(chan playerDrop),
		make(map[int]*hashCheck), 0, nil, make(chan *Envelope, RegistryQueueLength), nil, as, nil,
		db, collector, config}
}

// async api
func (r *Registry) CreatePlayer(m *Envelope) (inbox chan *Envelope, id int, master bool, mapURL string, errResp *Envelope) {
	pe := PlayerEnvelope{m, make(chan newPlayerReply, 1)}
	r.newPlayers <- pe
	response := <-pe.response
	return response.inbox, response.id, response.master, response.mapDownloadURL, response.errReply
}

func (r *Registry) RemovePlayer(id int, reason *Envelope) {
	r.droppedPlayers <- playerDrop{id, reason}
}

func (r *Registry) RecvMessage(e *Envelope) {
	r.inbox <- e
}

func (r *Registry) CheckVersion(version string) (bool, string) {
	req := versionCheck{version, make(chan versionCheckResponse, 1)}
	r.versionCheck <- req
	reply := <-req.response
	return reply.ok, reply.version
}

// internals
func (r *Registry) sendAll(m *Envelope) {
	var deadPlayers []int
	for idx := range r.clients {
		if !r.sendOne(idx, m) {
			deadPlayers = append(deadPlayers, idx)
		}
	}

	for _, idx := range deadPlayers {
		r.removePlayer(idx, NewEnvelope(&ErrmsgTooSlow{}, MsgidTooSlow, 0))
	}
}

func (r *Registry) sendOne(idx int, m *Envelope) bool {
	client := r.clients[idx]
	if client == nil {
		return false
	}

	select {
	case client <- m:
		return true
	default:
		return false
	}
}

func (r *Registry) sendMaster(m *Envelope) bool {
	if len(r.clients) == 0 {
		return false
	}

	for {
		select {
		case r.clients[r.masterID] <- m:
			return true
		default:
			// master is too slow, drop him
			// new master will be chosen in process of removal
			r.removePlayer(r.masterID, NewEnvelope(&ErrmsgTooSlow{}, MsgidTooSlow, 0))
			if r.masterID == -1 {
				return false
			}
		}
	}

	return false
}

func (r *Registry) Run() {
	r.ticker = time.NewTicker(TickInterval)

	for {
		select {
		case m := <-r.inbox:
			if r.handleHash(m) {
				continue
			}

			if r.handleRestart(m) {
				continue
			}

			if r.handleForcedNextTick(m) {
				continue
			}

			r.handleOOC(m)
			r.handleGameMessage(m)

		case now := <-r.ticker.C:
			r.handleNewTick(now)

		case newPlayer := <-r.newPlayers:
			r.registerPlayer(newPlayer)

		case info := <-r.droppedPlayers:
			r.removePlayer(info.id, info.reason)

		case req := <-r.versionCheck:
			r.handleVersionCheck(req)
		}
	}
}

func (r *Registry) checkForNewGame() {
	if len(r.clients) != 0 {
		return
	}

	// reset state for new game
	r.currentTick = 0
	sessionID := time.Now().Format(SessionDirTimeFormat)
	r.dumper = NewDumpWriter(r.config.DumpRoot, sessionID)
}

func (r *Registry) registerPlayer(newPlayer PlayerEnvelope) {
	m := newPlayer.m.Message.(*MessageLogin)
	var id int

	info, err := Authenticate(r.db, m.Login, m.Password, m.IsGuest)
	if err != nil {
		if err == ErrNotAuthenticated {
			log.Printf("registry: invalid auth for user '%s'", m.Login)
			e := NewEnvelope(&ErrmsgWrongAuth{}, MsgidWrongAuth, 0)
			response := newPlayerReply{errReply: e}
			newPlayer.response <- response
			return
		} else {
			log.Printf("registry: internal error while authenticating '%s': %v", m.Login, err)
			e := NewEnvelope(&ErrmsgInternalServerError{"cannot authenticate you"}, MsgidInternalServerError, 0)
			response := newPlayerReply{errReply: e}
			newPlayer.response <- response
			return
		}
	}

	if m.IsGuest {
		// generate new guest user
		info.Login = newGuest(r, m.Login)
	}

	if len(r.players) == 0 && !info.IsAdmin {
		// only admins allowed to start a game
		e := NewEnvelope(&ErrmsgNoMaster{}, MsgidNoMaster, 0)
		response := newPlayerReply{errReply: e}
		newPlayer.response <- response
		return
	}

	r.checkForNewGame()

	if r.clientVersion == "" {
		// the first valid client defines game version
		r.clientVersion = m.GameVersion
	}

	// look up for existing player avatar for current map
	if playerInfo, ok := r.players[info.Login]; ok {
		id = playerInfo.id
		log.Printf("registry: reusing existing id %d for player '%s'", id, info.Login)
		// remove player and then add him again
		r.removePlayer(id, nil)
		if _, ok := r.players[info.Login]; !ok {
			// server was restarted, so restart registration process
			r.registerPlayer(newPlayer)
			return
		}

		playerInfo = PlayerInfo{id: id, userInfo: info}
		r.players[info.Login] = playerInfo
	} else {
		id = r.nextID
		r.nextID++
		playerInfo = PlayerInfo{id: id, userInfo: info}
		r.players[info.Login] = playerInfo
		log.Printf("registry: registered new id %d for player '%s'", id, info.Login)

		// create player on maps
		newm := &MessageNewClient{&id}
		e := NewEnvelope(newm, MsgidNewClient, 0)
		r.sendAll(e)
		log.Printf("registry: creating new unit for client %d, login '%s'", id, info.Login)
	}

	// if there are queue for previous connection of client then close it
	if r.clients[id] != nil {
		close(r.clients[id])
		delete(r.clients, id)
	}

	// create inbox for client
	inbox := make(chan *Envelope, PlayerQueueLength)

	var master bool
	var mapDownloadURL string
	if r.masterID == -1 {
		// praise the new master!
		r.masterID = id
		master = true
		r.clients[id] = inbox
		log.Printf("registry: we have new master %d", id)
	} else {
		// postpone map request until new tick
		var mapUploadURL string
		_, mapUploadURL, mapDownloadURL = r.assetServer.MakePipe()

		requestMap := func(r *Registry) {
			if _, ok := r.players[info.Login]; !ok {
				close(inbox)
				return
			}

			curTick := r.currentTick
			m := &MessageMapUpload{&curTick, mapUploadURL}
			e := NewEnvelope(m, MsgidMapUpload, 0)
			log.Printf("registry: requesting master %d to send map", r.masterID)
			if r.sendMaster(e) {
				r.clients[id] = inbox
			} else {
				// drop connection
				close(inbox)
			}
		}

		r.onNextTick(requestMap)
	}

	r.collector.ObserveNewClient()

	response := newPlayerReply{id: id, master: master, mapDownloadURL: mapDownloadURL, inbox: inbox}
	newPlayer.response <- response
}

func (r *Registry) removePlayer(id int, reason *Envelope) {
	if reason != nil {
		log.Printf("registry: removing client %d with message: %s", id, reason.TypeName())
	} else {
		log.Printf("registry: removing client %d", id)
	}

	inbox, ok := r.clients[id]
	if !ok {
		// already deleted
		log.Printf("registry: client %d already removed", id)
		return
	}

	r.collector.ObserveDroppedClient()

	// send reason
	if reason != nil {
		r.sendOne(id, reason)
	}

	close(inbox)
	delete(r.clients, id)

	// if we deleted master, reelect new one
	if r.masterID == id {
		r.masterID = -1
		for cid := range r.clients {
			r.masterID = cid
			log.Printf("registry: switching master %d -> %d", id, cid)
			break
		}
	}

	if r.masterID == -1 {
		// there are no players left, clean up everything
		log.Println("registry: no players left, cleaning up")
		r.cleanUp()
	}
}

func (r *Registry) maybeSendConnCounter() {
	if r.currentTick%SendConnCountEvery != 0 {
		return
	}

	count := len(r.clients)
	msg := &MessageCurrentConnections{&count}
	e := NewEnvelope(msg, MsgidCurrentConnections, 0)
	r.sendAll(e)
}

func (r *Registry) onNextTick(callback func(r *Registry)) {
	r.nextTickCallbacks = append(r.nextTickCallbacks, callback)
}

func (r *Registry) invokeNextTickCallbacks() {
	for _, callback := range r.nextTickCallbacks {
		callback(r)
	}

	r.nextTickCallbacks = nil
}

func (r *Registry) handleHash(m *Envelope) bool {
	hash, ok := m.Message.(*MessageHash)
	if !ok {
		return false
	}

	checker := r.hashes[*hash.Tick]
	// too late hash check
	if checker == nil {
		return true
	}

	checker.add(m.From, hash)

	return true
}

func (r *Registry) handleRestart(m *Envelope) bool {
	if m.Kind != MsgidRestart {
		return false
	}

	player, ok := r.getPlayerByID(m.From)
	if !ok {
		return true
	}

	if !player.userInfo.IsAdmin {
		return true
	}

	// admin asked us to restart
	log.Printf("registry: admin %d (%s) asked to restart server", player.id, player.userInfo.Login)

	notifyMessage := NewEnvelope(&ErrmsgServerRestarting{}, MsgidServerRestarting, 0)

	// remove all players
	for _, playerInfo := range r.players {
		r.removePlayer(playerInfo.id, notifyMessage)
	}

	if len(r.clients) != 0 {
		// wtf
		ids := []string{}
		for id := range r.clients {
			ids = append(ids, strconv.Itoa(id))

			r.removePlayer(id, notifyMessage)
		}

		log.Printf("registry: some clients left after dropping all players: %s", strings.Join(ids, ", "))
	}

	return true
}

func (r *Registry) handleForcedNextTick(m *Envelope) bool {
	if m.Kind == MsgidNextTick {
		if user, ok := r.getPlayerByID(m.From); ok && user.userInfo.IsAdmin {
			r.handleNewTick(time.Now())
		}

		return true
	}

	return false
}

func (r *Registry) handleOOC(m *Envelope) {
	if m.Kind != MsgidOOCMessage {
		return
	}

	msg := m.Message.(*MessageOOC)
	info, ok := r.getPlayerByID(m.From)
	if ok {
		msg.Login = info.userInfo.Login
	} else {
		// wtf
		log.Printf("registry: failed to get user info for client %d", m.From)
		msg.Login = "<unknown>"
	}

	return
}

func (r *Registry) handleGameMessage(m *Envelope) {
	frv, ok := m.Message.(Forwardable)
	if !ok {
		log.Printf("registry: cannot set id for message while forwarding it, dropping. "+
			"ID: %d, type: %s", m.From, m.Message.TypeName())
		return
	}

	frv.SetID(m.From)
	r.sendAll(m)
}

func (r *Registry) handleNewTick(now time.Time) {
	r.sendNewTick()
	r.checkForHashStart(now)
	r.checkHashes(now)
	r.maybeSendConnCounter()
	r.invokeNextTickCallbacks()
}

func (r *Registry) sendNewTick() {
	r.currentTick++
	m := &MessageNewTick{}
	e := NewEnvelope(m, MsgidNewTick, 0)
	r.sendAll(e)
}

func (r *Registry) checkForHashStart(now time.Time) {
	if r.currentTick%StartHashCheckEvery != 0 || len(r.clients) < 2 {
		return
	}

	checker := &hashCheck{
		deadline:           now.Add(HashCheckTimeout),
		hashes:             make(map[int]int),
		waitingClientCount: len(r.clients),
	}

	r.hashes[r.currentTick] = checker
	// request hashes from all clients
	tick := r.currentTick
	m := &MessageRequestHash{&tick}
	e := NewEnvelope(m, MsgidRequestHash, 0)
	r.sendAll(e)
}

func (r *Registry) checkHashes(now time.Time) {
	for tick, checker := range r.hashes {
		if checker.deadline.Before(now) {
			// deadline expired for this hash check, force check
			// we shall use the tick when hash was computed but due to #201 we will
			// request current map
			r.checkHashesOne(r.currentTick, checker)
			delete(r.hashes, tick)
		} else if checker.waitingClientCount <= 0 {
			// we collected hashes from all clients, start check
			r.checkHashesOne(r.currentTick, checker)
			delete(r.hashes, tick)
		}
	}
}

func (r *Registry) checkHashesOne(tick int, checker *hashCheck) {
	master := r.masterID
	if master == -1 {
		// all of sudden we have no players, so nothing to do
		return
	}

	mhash, ok := checker.hashes[master]
	if !ok {
		// master is slow, very very slow
		log.Printf("hash-check: master %d did not sent his hash", master)
		return
	}
	delete(checker.hashes, master)

	unsynced := []int{}
	for id, hash := range checker.hashes {
		if hash != mhash {
			unsynced = append(unsynced, id)
		}
	}

	if len(unsynced) == 0 {
		// everyone in sync, OK
		return
	}

	unsyncedStrs := []string{}
	for _, id := range unsynced {
		unsyncedStrs = append(unsyncedStrs, strconv.Itoa(id))
	}

	log.Printf("hash-check: unsync detected on tick %d, fetching maps and dropping those clients: %s",
		tick, strings.Join(unsyncedStrs, ", "))

	// request and dump map from master and all of those faulty clients
	r.dumpPlayerMap(master, tick, nil)
	for _, id := range unsynced {
		r.dumpPlayerMap(id, tick, nil)
		r.removePlayer(id, NewEnvelope(&ErrmsgOutOfSync{}, MsgidOutOfSync, 0))
	}
}

func (r *Registry) dumpPlayerMap(id, tick int, callback func()) {
	pipe, uploadURL, _ := r.assetServer.MakePipe()
	mapreq := &MessageMapUpload{&tick, uploadURL}
	msg := NewEnvelope(mapreq, MsgidMapUpload, 0)
	if !r.sendOne(id, msg) {
		// too slow to live
		r.removePlayer(id, NewEnvelope(&ErrmsgTooSlow{}, MsgidTooSlow, 0))
		return
	}

	// start dumper
	uid := fmt.Sprintf("t%d-%d", tick, id)
	if id == r.masterID {
		uid += "-master"
	}

	go r.dumper.DumpMap(uid, pipe, callback)
}

func (r *Registry) handleVersionCheck(req versionCheck) {
	if r.clientVersion == "" {
		req.response <- versionCheckResponse{true, req.version}
	} else {
		req.response <- versionCheckResponse{req.version == r.clientVersion, r.clientVersion}
	}
}

func (r *Registry) cleanUp() {
	r.players = make(map[string]PlayerInfo)
	r.nextID = 1
	r.clientVersion = ""
	r.collector.ResetClients()
}

func (r *Registry) getPlayerByID(id int) (PlayerInfo, bool) {
	for _, p := range r.players {
		if p.id == id {
			return p, true
		}
	}
	return PlayerInfo{}, false
}

func newGuest(r *Registry, prefix string) string {
	for {
		next := rand.Intn(8192)
		name := prefix + strconv.Itoa(next)
		if _, ok := r.players[name]; !ok {
			return name
		}
	}
}
