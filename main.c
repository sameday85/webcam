#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "httpd.h"

#define CAM_IMAGE        "/tmp/today.jpg"
#define BUFF_SIZE       (64*1024) //64k

off_t fsize(const char *filename) {
    struct stat st;
    
    if (stat(filename, &st) == 0)
        return st.st_size;
    
    return -1;
}

int main(int c, char** v)
{
    serve_forever(c > 1 ? v[1] : "80");
    return 0;
}

void route()
{
    ROUTE_START()

    ROUTE_GET("/fswebcam")
    {
        //capture the picture
        char szCmd[255];
        sprintf(szCmd, "fswebcam -S 10 -r 1280x720 %s >/dev/null 2>&1", CAM_IMAGE);
        system(szCmd);
    
        off_t len = fsize(CAM_IMAGE);
        if (len > 0) {
            printf("HTTP/1.1 200 OK\r\n");
            printf("Content-Type: image/jpg\r\n");
            printf("Content-Disposition: attachment; filename=webcam.jpg");
            printf("Content-Length: %lld\r\n\r\n", len);
            //now the image data
            int fd = open(CAM_IMAGE, O_RDONLY);
            char *buff = (char *)malloc (BUFF_SIZE);
            int sz = read (fd, buff, BUFF_SIZE);
            while (sz > 0) {
                write (STDOUT_FILENO, buff, sz);
                sz = read (fd, buff, BUFF_SIZE);
            }
            free (buff);
            close (fd);
        }
        else {
            printf("HTTP/1.1 404 Not Found\r\n\r\n");
        }
    }
    ROUTE_END()
}
/* 
    $ch = curl_init('http://localhost:8080/fswebcam');
    $fp = fopen('/somewhere/upload/today.jpg', 'wb');
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_FILE, $fp);
    curl_exec($ch);
    curl_close($ch);
    fclose($fp);
*/
