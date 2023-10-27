#ifndef VIDEO_INFO_H
#define VIDEO_INFO_H

#include <string>
#include <vector>
#include <QObject>

struct VideoInfo {
    std::string path;
    std::string codec;
    double bitrate;
    double size;
    double ratio;
    int duration;
};

class VideoScanner : public QObject {
    Q_OBJECT
public:
    VideoScanner() {}
    ~VideoScanner() {}

    std::vector<VideoInfo> TraverseVideos(const std::string& directory, bool isRecursive);

signals:
    void progressUpdated(double progress);

private:
    bool IsValidVideoFile(const std::string& path);
    int CountSubfiles(const std::string& path);
    void GetVideoInfo(const std::wstring& path, std::vector<VideoInfo>& videoList);
};

#endif // VIDEO_INFO_H
