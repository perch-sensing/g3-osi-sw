#ifndef MAIN_HPP_
#define MAIN_HPP_
#include <string>

#define CHECK_CAMERA_COM "vcgencmd get_camera > camera_status.txt"
#define CAM_STAT_FLNM "camera_status.txt"
#define TAR_GPS_ERR_FILE_COM "tar -czvf gps-errors.tar.gz gps-errors.log"
#define TAR_ERR_FILE_COM "tar -czvf errors.tar.gz errors.log"
#define GPS_ERR_FILE_DUMMY "gps-errors-dummy.tar.gz"
#define ERR_FILE_DUMMY "errors-dummy.tar.gz"

#endif