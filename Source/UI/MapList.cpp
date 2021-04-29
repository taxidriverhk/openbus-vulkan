#include <string>

#include "Common/FileSystem.h"
#include "Config/ConfigReader.h"
#include "Config/MapConfig.h"
#include "MapList.h"

MapList::MapList()
{
    mapListContainer = std::make_unique<QTableView>(this);
    mapListItems = std::make_unique<MapTableModel>();

    mapListContainer->setModel(mapListItems.get());
    mapListContainer->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
    mapListContainer->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
    mapListContainer->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);
    mapListContainer->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeMode::ResizeToContents);

    setWindowTitle(TITLE);
    setWidget(mapListContainer.get());

    QItemSelectionModel *selectionModel = mapListContainer->selectionModel();
    connect(selectionModel, &QItemSelectionModel::selectionChanged, this, &MapList::MapListItemSelected);

    Refresh();
}

MapList::~MapList()
{
}

bool MapList::GetSelectedMapFile(std::string &mapFilePath)
{
    QItemSelectionModel *selectionModel = mapListContainer->selectionModel();
    int selectedRow = selectionModel->currentIndex().row();
    if (selectedRow < 0)
    {
        return false;
    }

    mapFilePath = mapListItems->GetMapPathByIndex(selectedRow);
    return true;
}

void MapList::Refresh()
{
    std::string mapDirectory = FileSystem::MapDirectory();
    for (const auto &mapEntry : std::filesystem::directory_iterator(mapDirectory))
    {
        if (mapEntry.is_directory())
        {
            // Add map to the list only if the directory contains map.json file
            for (const auto &mapFile : std::filesystem::directory_iterator(mapEntry))
            {
                if (mapFile.is_regular_file() && mapFile.path().filename() == FileSystem::MAP_FILE_NAME)
                {
                    MapInfoConfig mapConfig;
                    ConfigReader::ReadConfig(mapFile.path().string(), mapConfig);

                    std::filesystem::path imagePath = mapEntry.path() / mapConfig.image;
                    mapListItems->InsertMapTableRow(mapFile.path().string(), mapConfig.name, imagePath.string());
                }
            }
        }
    }
}

int MapTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mapNames.count();
}

int MapTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 2;
}

QVariant MapTableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    int row = index.row();
    int column = index.column();
    if (column == 0 && role == Qt::DecorationRole)
    {
        if (mapThumbnails[row].isNull())
        {
            return QVariant();
        }
        return mapThumbnails[row];
    }
    else if (column == 1 && role == Qt::DisplayRole)
    {
        return mapNames[row];
    }

    return QVariant();
}


QVariant MapTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
    {
        if (section == 0)
        {
            return QString("Thumbnail");
        }
        else if (section == 1)
        {
            return QString("Name");
        }
    }
    return QVariant();
}

std::string MapTableModel::GetMapPathByIndex(int index)
{
    return mapPaths[index];
}

void MapTableModel::InsertMapTableRow(std::string mapPath, std::string mapName, std::string imagePath)
{
    QString name = QString::fromStdString(mapName);
    QPixmap pixmap(QString::fromStdString(imagePath));
    pixmap = pixmap.scaledToHeight(THUMBNAIL_MIN_HEIGHT_PX);
    pixmap = pixmap.scaledToWidth(THUMBNAIL_MIN_WIDTH_PX);

    mapNames.append(name);
    mapThumbnails.append(pixmap);
    mapPaths.push_back(mapPath);
}
