#pragma once

namespace MessageType
{
    // MESSAGES TO SERVER FROM CLIENT:
    const int INITAL_LOGIN_MESSAGE = 1;
    const int EXIT_CLIENT = 2;
    const int HASH_MESSAGE = 3;
    const int RESTART_ROUND = 4;
    const int NEXT_TICK = 5;

    // MESSAGES TO CLIENT FROM SERVER:

    // ERROR MESSAGES:
    const int WRONG_GAME_VERSION = 401;
    const int WRONG_AUTHENTICATION = 402;
    const int UNDEFINED_ERROR = 403;
    const int EXIT_SERVER = 404;
    const int MASTER_CLIENT_IS_NOT_CONNECTED = 405;
    const int CLIENT_IS_OUT_OF_SYNC = 406;
    const int CLIENT_TOO_SLOW = 407;
    const int INTERNAL_SERVER_ERROR = 408;
    const int SERVER_IS_RESTARTING = 409;

    // NON ERROR MESSAGES:
    const int SUCCESS_CONNECTION = 201;
    const int MAP_UPLOAD = 202;
    const int NEW_TICK = 203;
    const int NEW_CLIENT = 204;
    const int CURRENT_CONNECTIONS = 205;
    const int REQUEST_HASH = 206;

    // GAME MESSAGES
    const int ORDINARY = 1001;
    const int MESSAGE = 1002;

    const int MOUSE_CLICK = 1004;
    const int OOC_MESSAGE = 1005;

    //const int HASH = 1101;
    const int PING = 1102;
}
