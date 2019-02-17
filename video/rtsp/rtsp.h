#ifndef RTSP
  #define RTSP

  enum RtspMethods{
    RTSP_METHOD_ERROR=-1,
    RTSP_METHOD_OPTIONS=0,
    RTSP_METHOD_DESCRIBE=1,
    RTSP_METHOD_PLAY=2,
    RTSP_METHOD_PAUSE=3,
    RTSP_METHOD_RECORD=4,
    RTSP_METHOD_REDIRECT=5,
    RTSP_METHOD_SETUP=6,
    RTSP_METHOD_ANNOUNCE=7,
    RTSP_METHOD_GET_PARAMETER=8,
    RTSP_METHOD_SET_PARAMETER=9,
    RTSP_METHOD_TEARDOWN=10
  };

  enum RtspResults {
    RTSP_RESULT_OK = 200,
    RTSP_RESULT_CLIENT_ERROR = 400,
    RTSP_RESULT_PARAM_ERROR = 451
  };

  struct Field{
    char *header;
    char *value;
  };

  typedef struct RtspMessage_{
    enum RtspMethods method;
    char *URI;
    int cseq;
    int rtspVerMinor,rtspVerMajor;
    int fieldsCount;
    struct Field *fields;
    char* content;
  } RtspMessage;

  typedef struct RtspAnswer_{
    enum RtspResults code;
    int rtspVerMinor,rtspVerMajor;
    int fieldsCount;
    struct Field *fields;
  } RtspAnswer;

  int rtsp_server(int fd);
  int rtsp_answer(RtspMessage *rtsp, enum RtspResults res, int fd);

#endif
