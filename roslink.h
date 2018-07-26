#ifndef ROSLINK_H
#define ROSLINK_H

#include "geographicsitem.h"

#include "geographic_msgs/GeoPointStamped.h"
#include "sensor_msgs/NavSatFix.h"
#include "marine_msgs/NavEulerStamped.h"
#include "marine_msgs/Heartbeat.h"
#include "ros/ros.h"
#include "asv_msgs/AISContact.h"
#include "std_msgs/String.h"


Q_DECLARE_METATYPE(ros::Time);

class ROSDetails;

struct ROSAISContact: public QObject
{
    Q_OBJECT
public:
    ROSAISContact(QObject *parent = nullptr);
    uint32_t mmsi;
    std::string name;
    QGeoCoordinate location;
    QPointF location_local;
    double heading;
    float dimension_to_stbd; 
    float dimension_to_port;
    float dimension_to_bow;
    float dimension_to_stern; 
};


class ROSLink : public QObject, GeoGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    ROSLink(AutonomousVehicleProject* parent);
    
    QRectF boundingRect() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QPainterPath shape() const override;
    QPainterPath vehicleShape() const;
    QPainterPath vehicleShapePosmv() const;
    QPainterPath baseShape() const;
    QPainterPath aisShape() const;
    QPainterPath viewShape() const;

    void write(QJsonObject &json) const;
    void read(const QJsonObject &json);
    
    int type() const {return ROSLinkType;}
    
    bool active() const;
    void setActive(bool active);
    
    std::string const &helmMode() const;
    void setHelmMode(const std::string& helmMode);
    void setROSDetails(ROSDetails *details);

    
signals:

    void rosConnected(bool connected);
    void originUpdated();
    
public slots:
    void updateLocation(QGeoCoordinate const &location);
    void updatePosmvLocation(QGeoCoordinate const &location);
    void updateBaseLocation(QGeoCoordinate const &location);
    void updateOriginLocation(QGeoCoordinate const &location);
    void updateHeading(double heading);
    void updatePosmvHeading(double heading);
    void updateBaseHeading(double heading);
    void updateBackground(BackgroundRaster *bgr);
    void recalculatePositions();
    void addAISContact(ROSAISContact *c);
    void sendWaypoints(QList<QGeoCoordinate> const &waypoints);
    void sendLoiter(QGeoCoordinate const &loiterLocation);
    void sendGoto(QGeoCoordinate const &loiterLocation);
    void connectROS();
    void updateHeartbeatTimes(ros::Time const &last_heartbeat_timestamp, ros::Time const &last_heartbeat_receive_time);
    void watchdogUpdate();
    void updateMapScale(qreal scale);
    
private:
    void geoPointStampedCallback(const geographic_msgs::GeoPointStamped::ConstPtr& message);
    void baseNavSatFixCallback(const sensor_msgs::NavSatFix::ConstPtr& message);
    void originCallback(const geographic_msgs::GeoPoint::ConstPtr& message);
    void headingCallback(const marine_msgs::NavEulerStamped::ConstPtr& message);
    void baseHeadingCallback(const marine_msgs::NavEulerStamped::ConstPtr& message);
    void aisCallback(const asv_msgs::AISContact::ConstPtr& message);
    void heartbeatCallback(const marine_msgs::Heartbeat::ConstPtr& message);
    void viewPointCallback(const std_msgs::String::ConstPtr&message);
    void viewPolygonCallback(const std_msgs::String::ConstPtr&message);
    void viewSeglistCallback(const std_msgs::String::ConstPtr&message);
    void posmvOrientationCallback(const marine_msgs::NavEulerStamped::ConstPtr& message);
    void posmvPositionCallback(const sensor_msgs::NavSatFix::ConstPtr& message);
    
    void drawTriangle(QPainterPath &path, QGeoCoordinate const &location, double heading_degrees, double scale=1.0) const;
    void drawShipOutline(QPainterPath &path, QGeoCoordinate const &location, double heading_degrees, float dimension_to_bow, float dimension_to_port, float dimension_to_stbd, float dimension_to_stern) const;
    QMap<QString,QString> parseViewString(QString const &vs) const;
    QList<QPointF> parseViewPointList(QString const &pointList) const;
    
    QGeoCoordinate rosMapToGeo(QPointF const &location) const;
    
    AutonomousVehicleProject *autonomousVehicleProject() const;
    
    ros::NodeHandle *m_node;
    ros::Subscriber m_geopoint_subscriber;
    ros::Subscriber m_base_navsatfix_subscriber;
    ros::Subscriber m_origin_subscriber;
    ros::Subscriber m_heading_subscriber;
    ros::Subscriber m_base_heading_subscriber;
    ros::Subscriber m_ais_subscriber;
    ros::Subscriber m_heartbeat_subscriber;
    ros::Subscriber m_view_point_subscriber;
    ros::Subscriber m_view_polygon_subscriber;
    ros::Subscriber m_view_seglist_subscriber;
    ros::Subscriber m_posmv_position;
    ros::Subscriber m_posmv_orientation;
    
    ros::Publisher m_active_publisher;
    ros::Publisher m_helmMode_publisher;
    ros::Publisher m_wpt_updates_publisher;
    ros::Publisher m_loiter_updates_publisher;
    ros::AsyncSpinner *m_spinner;
    QGeoCoordinate m_location;
    QGeoCoordinate m_posmv_location;
    QGeoCoordinate m_base_location; // location of the base operator station (ship, shore station, etc)
    QGeoCoordinate m_origin;
    std::vector<QGeoCoordinate> m_location_history;
    std::list<QPointF> m_local_location_history;
    std::vector<QGeoCoordinate> m_posmv_location_history;
    std::list<QPointF> m_local_posmv_location_history;
    std::vector<QGeoCoordinate> m_base_location_history;
    std::list<QPointF> m_local_base_location_history;
    QPointF m_local_reference_position;
    bool m_have_local_reference;
    double m_heading;
    double m_posmv_heading;
    double m_base_heading;
    bool m_active;
    std::string m_helmMode;
    
    typedef std::list<ROSAISContact*> ContactList;
    typedef std::map<uint32_t,ContactList> ContactMap;
    
    ContactMap m_contacts;
    ROSDetails *m_details;
    
    QGeoCoordinate m_view_point;
    QPointF m_local_view_point;
    bool m_view_point_active;
    
    QList<QGeoCoordinate> m_view_seglist;
    QList<QPointF> m_local_view_seglist;
    bool m_view_seglist_active;
    
    QList<QGeoCoordinate> m_view_polygon;
    QList<QPointF> m_local_view_polygon;
    bool m_view_polygon_active;
    
    ros::Time m_last_heartbeat_timestamp;
    ros::Time m_last_heartbeat_receive_time;
    
    QTimer * m_watchdog_timer;
    
    qreal m_map_scale;
    qreal m_pixel_size;
};

#endif // ROSNODE_H
