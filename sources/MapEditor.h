#pragma once

#include <string>
#include <vector>
#include <set>
#include <map>

#include <QObject>
#include <QPixmap>
#include <QGraphicsPixmapItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsSceneMouseEvent>
#include <QKeyEvent>
#include <QVector>

class MapEditor: public QObject
{
    Q_OBJECT
public:
    struct EditorEntry
    {
        EditorEntry();

        QString item_type;

        std::map<QString, QString> variables;

        QGraphicsPixmapItem* pixmap_item;
    };

    struct EditorTile
    {
        EditorEntry turf;
        std::vector<EditorEntry> items;
    };

    struct Pointer
    {
        Pointer();

        int first_posx;
        int first_posy;

        int second_posx;
        int second_posy;

        QGraphicsPolygonItem* image;
    };

    MapEditor(QGraphicsScene* scene);

    void SaveMapgen(const QString& name);

    void LoadMapgen(const QString& name);

    void fix_borders(int* posx, int* posy);

    void SetPointer(int posx, int posy);

    void Resize(int posx, int posy, int posz);

    void AddItemType(const QString& item_type, QVector<QPixmap> images);

    void AddTurfType(const QString& item_type);

    void AddItem(const QString& item_type);

    void UpdateDirs(EditorEntry* ee);

    void RemoveItems();

    void RemoveItems(int posx, int posy, int posz);

    MapEditor::EditorEntry& AddItem(const QString& item_type, int posx, int posy, int posz);

    void SetTurf(const QString& item_type);

    MapEditor::EditorEntry& SetTurf(const QString& item_type, int posx, int posy, int posz);

    void SetSelectionStage(int stage);
    int GetSelectionStage() const;

    void SetFirstSelectionPoint(int x, int y);

    void SetSecondSelectionPoint(int x, int y);

    void ClearMap();

    std::vector<EditorEntry>& GetEntriesFor(int posx, int posy, int posz);

    const Pointer& GetPointer() const { return pointer_; }
signals:
    void newSelectionSetted(int first_x, int first_y, int second_x, int second_y);
public slots:
    void mousePressedEvent(QGraphicsSceneMouseEvent* mouseEvent);
    void keyPressedEvent(QKeyEvent* event);
private:
    std::vector<EditorEntry> copypaste_items_;

    void CopyItemsFromCurrentTile();
    void PasteItemsToCurrentTile();

    std::vector<std::vector<EditorTile>> area_buffer_;

    void CopyToAreaBuffer();
    void PasteFromAreaBuffer();


    int selection_stage_;

    int first_selection_x_;
    int first_selection_y_;

    int second_selection_x_;
    int second_selection_y_;

    QGraphicsPolygonItem* border_image_;

    Pointer pointer_;

    std::map<QString, QVector<QPixmap>> image_holder_;

    std::set<QString> turf_types_;

    QGraphicsScene* scene_;

    typedef std::vector<std::vector<std::vector<EditorTile>>> MapType;
    MapType editor_map_;
};
