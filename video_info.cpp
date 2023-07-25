#include "video_info.h"
#include <QDir>
#include <QDirIterator>
#include <QString>
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

int VideoScanner::CountSubfiles(const std::string& path) {
    int cnt = 0;
    QDir dir(QString::fromStdString(path));
    QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);
    while (it.hasNext()) {
        it.next();
        if (!it.fileInfo().isDir() && IsValidVideoFile(it.filePath().toStdString())) {
            cnt++;
        }
    }
    return cnt;
}

std::vector<VideoInfo> VideoScanner::TraverseVideos(const std::string& directory) {
    std::vector<VideoInfo> videoList;
    QDir dir(QString::fromStdString(directory));

    QDirIterator it(dir.absolutePath(), QDirIterator::Subdirectories);

    int totalFiles = CountSubfiles(directory);
    int processedFiles = 0;

    while (it.hasNext()) {
        it.next();
        if (!it.fileInfo().isDir() && IsValidVideoFile(it.filePath().toStdString())) {
            GetVideoInfo(it.filePath().toStdWString(), videoList);
            processedFiles++;
            emit progressUpdated((double)processedFiles / totalFiles);
        }
    }
    return videoList;
}


bool VideoScanner::IsValidVideoFile(const std::string& path) {
    const std::string videoExtensions[] = { ".mp4", ".avi", ".mov", ".flv", ".wmv", ".mkv", ".mpg" };
    std::string extension = path.substr(path.find_last_of('.'));
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    for (const auto& videoExtension : videoExtensions) {
        if (extension == videoExtension) {
            return true;
        }
    }

    return false;
}

void VideoScanner::GetVideoInfo(const std::wstring& path, std::vector<VideoInfo>& videoList) {
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    std::string realpath = QString::fromStdWString(path).toStdString();
    if (avformat_open_input(&pFormatCtx, realpath.c_str(), NULL, NULL) != 0) {
        return;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
        return;
    }

    int videoStream = -1;
    for (int i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        avformat_close_input(&pFormatCtx);
        return;
    }

    AVCodec* pCodec = const_cast<AVCodec*>(avcodec_find_decoder(
        pFormatCtx->streams[videoStream]->codecpar->codec_id));
    double bitrate = pFormatCtx->streams[videoStream]->codecpar->bit_rate / 1024.0;
    double size = static_cast<double>(QFileInfo(QString::fromStdWString(path)).size()) / (1024.0 * 1024.0);
    double ratio = (bitrate > 0.0) ? (size * size / bitrate) : 0.0;
    int duration = static_cast<int>(pFormatCtx->duration / AV_TIME_BASE);

    VideoInfo videoInfo;
    videoInfo.path = realpath;
    videoInfo.codec = (pCodec == nullptr ? "Unsupported" : pCodec->long_name);
    videoInfo.bitrate = bitrate;
    videoInfo.size = size;
    videoInfo.ratio = ratio;
    videoInfo.duration = (duration >= 0) ? duration : 0;

    videoList.push_back(videoInfo);
    avformat_close_input(&pFormatCtx);
}
