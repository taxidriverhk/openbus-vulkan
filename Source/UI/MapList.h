#pragma once

#include <memory>
#include <vector>

#include <QtWidgets>

class MapTableModel;

class MapList : public QDockWidget
{
    Q_OBJECT

public:
    MapList();
    ~MapList();

    bool GetSelectedMapFile(std::string &mapFilePath);

Q_SIGNALS:
    void MapListItemSelected();

private:
    static constexpr char *TITLE = "Map List";

    void Refresh();

    std::unique_ptr<QTableView> mapListContainer;
    std::unique_ptr<MapTableModel> mapListItems;
};

class MapTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    int rowCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;
    int columnCount(const QModelIndex &parent = QModelIndex()) const Q_DECL_OVERRIDE;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    std::string GetMapPathByIndex(int index);
    void InsertMapTableRow(std::string mapPath, std::string mapName, std::string imagePath);

private:
    static constexpr int THUMBNAIL_MIN_HEIGHT_PX = 200;
    static constexpr int THUMBNAIL_MIN_WIDTH_PX = 200;

    QList<QString> mapNames;
    QList<QPixmap> mapThumbnails;
    std::vector<std::string> mapPaths;
};
