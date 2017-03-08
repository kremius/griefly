#include "mapeditorform.h"
#include "ui_mapeditorform.h"

#include "core/ObjectFactory.h"
#include "core/Map.h"
#include "representation/Text.h"

#include "representation/View2.h"
#include "representation/SpriteHolder.h"

#include <map>
#include <set>

#include <QBitmap>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QDebug>
#include <QGraphicsSceneMouseEvent>
#include <QFileDialog>
#include <QInputDialog>

#include "AutogenMetadata.h"
#include "core/StreamWrapper.h"

GraphicsScene::GraphicsScene(QWidget *parent) : QGraphicsScene(parent)
{

}

void GraphicsScene::mousePressEvent(QGraphicsSceneMouseEvent* mouseEvent)
{
    emit mousePressed(mouseEvent);
    if (mouseEvent->button() == Qt::RightButton)
    {
        emit rightClick();
    }
}

void GraphicsScene::keyPressEvent(QKeyEvent *event)
{
    emit keyboardPressed(event);
}

MapEditorForm::MapEditorForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MapEditorForm)
{
    ui->setupUi(this);

    is_turf_selected_ = false;

    InitSettersForTypes();

    scene_ = new GraphicsScene;
    map_editor_ = new MapEditor(scene_);

    connect(scene_, &GraphicsScene::mousePressed, map_editor_, &MapEditor::mousePressedEvent);
    connect(scene_, &GraphicsScene::keyboardPressed, map_editor_, &MapEditor::keyPressedEvent);
    connect(scene_, &GraphicsScene::rightClick, this, &MapEditorForm::mapClicked);
    connect(map_editor_, &MapEditor::newSelectionSetted, this, &MapEditorForm::newSelectionSetted);

    map_editor_->Resize(100, 100, 1);

    map_editor_->SetPointer(2, 2);

    ui->graphicsView->setScene(scene_);

    SetSpriter(new SpriteHolder);

    qDebug() << "Start generate images for creators";
    for (auto it = (*GetItemsCreators()).begin(); it != (*GetItemsCreators()).end(); ++it)
    {
        IMainObject* loc = it->second(0);
        IOnMapObject* bloc = CastTo<IOnMapObject>(loc);
        if (!bloc)
        {
            qDebug() << it->first;
            continue;
        }
        bool is_turf = false;
        if (CastTo<ITurf>(loc))
        {
            is_turf = true;
        }

        ViewInfo* view_info = bloc->GetView();

        qDebug() << view_info;

        if (   view_info->GetBaseFrameset().GetSprite().isEmpty()
            || view_info->GetBaseFrameset().GetState().isEmpty())
        {
            qDebug() << "EMPTY frameset:" << it->first;
            continue;
        }

        QVector<QPixmap> images;

        View2 view;
        view.LoadViewInfo(*view_info);

        if (view.GetBaseFrameset().GetMetadata() == nullptr)
        {
            qDebug() << "EMPTY metadata:" << it->first;
            continue;
        }

		int FrameW = view.GetBaseFrameset().GetSprite()->FrameW();
		int FrameH = view.GetBaseFrameset().GetSprite()->FrameH();
        for (quint32 dir = 0; dir < view.GetBaseFrameset().GetMetadata()->dirs; ++dir)
        {
            int current_frame_pos = view.GetBaseFrameset().GetMetadata()->first_frame_pos + dir;
			int image_state_h_ = current_frame_pos / FrameW;
            int image_state_w_ = current_frame_pos % FrameW;

            QImage img = view.GetBaseFrameset().GetSprite()->GetSDLSprite()->frames
                    [image_state_w_ * FrameH  + image_state_h_];

            images.push_back(QPixmap::fromImage(img));
        }
        map_editor_->AddItemType(it->first, images);

        if (images.length() == 0)
        {
            qDebug() << images.length();
        }

        QListWidgetItem* new_item
                = new QListWidgetItem(QIcon(images[0]), bloc->T_ITEM());

        if (!is_turf)
        {
            types_.push_back(it->first);
            ui->listWidget->addItem(new_item);
        }
        else
        {
            turf_types_.push_back(it->first);
            map_editor_->AddTurfType(it->first);
            ui->listWidgetTurf->addItem(new_item);
        }
    }
    qDebug() << "End generating";
}

MapEditorForm::~MapEditorForm()
{
    delete ui;
    delete scene_;
    delete map_editor_;
}

void MapEditorForm::newSelectionSetted(int first_x, int first_y, int second_x, int second_y) // sceond_x , second_y not used ehm??
{
    ui->cursor_label->setText(tr("(%1, %2)").arg(first_x).arg(first_y));

    auto entries = map_editor_->GetEntriesFor(first_x, first_y, 0);

    ui->listWidgetTile->clear();
    for (auto it = entries.begin(); it != entries.end(); ++it)
    {
        ui->listWidgetTile->addItem(it->item_type);
    }

    ui->listWidgetVariables->clear();
    ui->lineEditAsString->clear();
    ui->lineEditRaw->clear();

    on_listWidgetTile_itemSelectionChanged();
}

void MapEditorForm::on_createItem_clicked()
{
    int current_row = ui->listWidget->currentRow();
    if (current_row < 0)
    {
        return;
    }
    QString type = types_[current_row];
    map_editor_->AddItem(type);
}

void MapEditorForm::mapClicked()
{
    if (is_turf_selected_)
    {
        on_createTurf_clicked();
    }
    else
    {
        on_createItem_clicked();
    }
}

void MapEditorForm::on_createTurf_clicked()
{
    int current_row = ui->listWidgetTurf->currentRow();
    if (current_row < 0)
    {
        return;
    }
    QString type = turf_types_[current_row];
    map_editor_->SetTurf(type);
}

void MapEditorForm::on_beginSelection_clicked()
{
    map_editor_->SetSelectionStage(1);
}

void MapEditorForm::on_removeItem_clicked()
{
    map_editor_->RemoveItems();
}

void MapEditorForm::on_newMap_clicked()
{
    bool ok = false;

    int size_x = ui->posxEdit->text().toInt(&ok);
    if (!ok || (size_x <= 0))
    {
        return;
    }
    int size_y = ui->posyEdit->text().toInt(&ok);
    if (!ok || (size_y <= 0))
    {
        return;
    }
    int size_z = ui->poszEdit->text().toInt(&ok);
    if (!ok || (size_z <= 0))
    {
        return;
    }
    map_editor_->ClearMap();
    map_editor_->Resize(size_x, size_y, size_z);
}

void MapEditorForm::on_saveMap_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilter(tr("Mapgen files (*.gen.txt)"));
    QStringList file_names;
    if (!dialog.exec())
    {
        return;
    }

    file_names = dialog.selectedFiles();
    map_editor_->SaveMapgen(file_names[0]);
}

void MapEditorForm::on_loadMap_clicked()
{
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter(tr("Mapgen files (*.gen.txt)"));
    QStringList file_names;
    if (!dialog.exec())
    {
        return;
    }

    file_names = dialog.selectedFiles();
    map_editor_->LoadMapgen(file_names[0]);
}

void MapEditorForm::on_listWidgetTile_itemSelectionChanged()
{
    QString name;
    if (QListWidgetItem* item = ui->listWidgetVariables->currentItem())
    {
        name = item->text();
    }

    ui->listWidgetVariables->clear();

    QString item_type = GetCurrentEditorEntry()->item_type;

    auto& variables = GetSettersForTypes()[item_type];

    int counter = 0;
    for (auto it = variables.begin(); it != variables.end(); ++it, ++counter)
    {
        ui->listWidgetVariables->addItem(it->first);
        if (it->first == name)
        {
            ui->listWidgetVariables->setCurrentRow(counter);
        }
    }

    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (ee)
    {
        UpdateVariablesColor(*ee);
    }
}

MapEditor::EditorEntry* MapEditorForm::GetCurrentEditorEntry()
{
    int current_index = ui->listWidgetTile->currentRow();

    int current_x = map_editor_->GetPointer().first_posx;
    int current_y = map_editor_->GetPointer().first_posy;

    if (ui->listWidgetTile->selectedItems().size() == 0)
    {
        return &map_editor_->GetTurfFor(current_x, current_y, 0);
    }

    auto& entries = map_editor_->GetEntriesFor(current_x, current_y, 0);
    if (entries.size() > static_cast<quint32>(current_index))
    {
        return &entries[current_index];
    }
    return nullptr;
}

void MapEditorForm::UpdateVariablesColor(MapEditor::EditorEntry& ee)
{
    for (int i = 0; i < ui->listWidgetVariables->count(); ++i)
    {
        if (ee.variables[ui->listWidgetVariables->item(i)->text()].size())
        {
            ui->listWidgetVariables->item(i)->setBackgroundColor(QColor(200, 150, 170));
        }
        else
        {
            ui->listWidgetVariables->item(i)->setBackgroundColor(QColor(255, 255, 255));
        }
    }
}

void MapEditorForm::on_listWidgetVariables_itemSelectionChanged()
{
    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (!ee)
    {
        return;
    }

    QByteArray& variable_value = ee->variables[ui->listWidgetVariables->currentItem()->text()];

    ui->lineEditRaw->setText(variable_value.toHex());

    {
        FastDeserializer deserializer(variable_value.data(), variable_value.size());

        QString parsed_value("PARSING_ERROR");
        if (deserializer.IsNextType(FastSerializer::STRING_TYPE))
        {
            WrapReadMessage(deserializer, parsed_value);
        }

        ui->lineEditAsString->setText(parsed_value);
    }

    {
        FastDeserializer deserializer(variable_value.data(), variable_value.size());

        QString parsed_value("PARSING_ERROR");
        if (deserializer.IsNextType(FastSerializer::INT32_TYPE))
        {
            int value;
            WrapReadMessage(deserializer, value);
            parsed_value = QString::number(value);
        }

        ui->lineEditAsInt->setText(parsed_value);
    }
    {
        FastDeserializer deserializer(variable_value.data(), variable_value.size());

        QString parsed_value("PARSING_ERROR");
        if (deserializer.IsNextType(FastSerializer::BOOL_TYPE))
        {
            bool value;
            WrapReadMessage(deserializer, value);
            parsed_value = value ? "1" : "0";
        }

        ui->lineEditAsBool->setText(parsed_value);
    }
}

void MapEditorForm::on_lineEditRaw_returnPressed()
{
    if (!ui->listWidgetVariables->currentItem())
    {
        return;
    }

    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (!ee)
    {
        return;
    }

    QByteArray& variable_value = ee->variables[ui->listWidgetVariables->currentItem()->text()];

    variable_value = QByteArray::fromHex(ui->lineEditRaw->text().toUtf8());

    on_listWidgetVariables_itemSelectionChanged();
    UpdateVariablesColor(*ee);

    map_editor_->UpdateDirs(ee);
}

void MapEditorForm::on_lineEditAsString_returnPressed()
{
    if (!ui->listWidgetVariables->currentItem())
    {
        return;
    }

    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (!ee)
    {
        return;
    }

    QString current_variable = ui->listWidgetVariables->currentItem()->text();

    FastSerializer ss(1);
    QString loc = ui->lineEditAsString->text();
    WrapWriteMessage(ss, loc);

    ee->variables[current_variable] = QByteArray(ss.GetData(), ss.GetIndex());

    on_listWidgetVariables_itemSelectionChanged();
    UpdateVariablesColor(*ee);
}

void MapEditorForm::on_lineEditAsInt_returnPressed()
{
    if (!ui->listWidgetVariables->currentItem())
    {
        return;
    }

    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (!ee)
    {
        return;
    }

    QString current_variable = ui->listWidgetVariables->currentItem()->text();

    FastSerializer ss(1);
    QString loc = ui->lineEditAsInt->text();

    bool ok = false;
    int value = loc.toInt(&ok);
    if (!ok)
    {
        return;
    }

    WrapWriteMessage(ss, value);

    ee->variables[current_variable] = QByteArray(ss.GetData(), ss.GetIndex());

    on_listWidgetVariables_itemSelectionChanged();
    UpdateVariablesColor(*ee);

    map_editor_->UpdateDirs(ee);
}

void MapEditorForm::on_lineEditAsBool_returnPressed()
{
    if (!ui->listWidgetVariables->currentItem())
    {
        return;
    }

    MapEditor::EditorEntry* ee = GetCurrentEditorEntry();
    if (!ee)
    {
        return;
    }

    QString current_variable = ui->listWidgetVariables->currentItem()->text();

    FastSerializer ss(1);
    QString loc = ui->lineEditAsBool->text();

    bool ok = false;
    bool value = !!loc.toInt(&ok);
    if (!ok)
    {
        return;
    }

    WrapWriteMessage(ss, value);

    ee->variables[current_variable] = QByteArray(ss.GetData(), ss.GetIndex());

    on_listWidgetVariables_itemSelectionChanged();
    UpdateVariablesColor(*ee);
}

void MapEditorForm::on_listWidgetTurf_clicked(const QModelIndex&index)
{
    is_turf_selected_ = true;
}

void MapEditorForm::on_listWidget_clicked(const QModelIndex &index)
{
    is_turf_selected_ = false;
}

void MapEditorForm::on_resizeMap_clicked()
{
    bool ok = false;

    int size_x = ui->posxEdit->text().toInt(&ok);
    if (!ok || (size_x <= 0))
    {
        return;
    }
    int size_y = ui->posyEdit->text().toInt(&ok);
    if (!ok || (size_y <= 0))
    {
        return;
    }
    int size_z = ui->poszEdit->text().toInt(&ok);
    if (!ok || (size_z <= 0))
    {
        return;
    }
    map_editor_->Resize(size_x, size_y, size_z);
}
