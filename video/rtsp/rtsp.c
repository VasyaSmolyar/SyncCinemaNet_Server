#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

#include "rtsp.h"
#include "../interfaces/errors.h"

#define BUF_SIZE 1024

extern int errno;

int rtsp_server(int fd) {
  enum RtspResults res;
  char *buf, *tok;
  buf=(char *) malloc(sizeof(char)*BUF_SIZE);
  int requestSize = read(fd,buf,BUF_SIZE);
  RtspMessage msg;

  if(requestSize == -1) {
    return 1;
  }else if(requestSize == 0){
    msg.rtspVerMajor = 1;
    msg.rtspVerMinor = 0;
    res = RTSP_RESULT_CLIENT_ERROR;
    return rtsp_answer(&msg, res, fd);
    //strcpy(wbuf,"RTSP/1.0 400 Bad Request\0");
  }else{
    //parse
    tok=strtok(buf," \n");//Method
    if (tok == NULL) {
      msg.rtspVerMajor = 1;
      msg.rtspVerMinor = 0;
      res = RTSP_RESULT_CLIENT_ERROR;
      return rtsp_answer(&msg, res, fd);
    }
    if(!strcmp(tok,"OPTIONS")){
      msg.method=RTSP_METHOD_OPTIONS;
    }else if(!strcmp(tok,"DESCRIBE")){
      msg.method=RTSP_METHOD_DESCRIBE;
    }else if(!strcmp(tok,"PLAY")){
      msg.method=RTSP_METHOD_PLAY;
    }else if(!strcmp(tok,"PAUSE")){
      msg.method=RTSP_METHOD_PAUSE;
    }else if(!strcmp(tok,"RECORD")){
      msg.method=RTSP_METHOD_RECORD;
    }else if(!strcmp(tok,"REDIRECT")){
      msg.method=RTSP_METHOD_REDIRECT;
    }else if(!strcmp(tok,"SETUP")){
      msg.method=RTSP_METHOD_SETUP;
    }else if(!strcmp(tok,"ANNOUNCE")){
      msg.method=RTSP_METHOD_ANNOUNCE;
    }else if(!strcmp(tok,"GET_PARAMETER")){
      msg.method=RTSP_METHOD_GET_PARAMETER;
    }else if(!strcmp(tok,"SET_PARAMETER")){
      msg.method=RTSP_METHOD_SET_PARAMETER;
    }else if(!strcmp(tok,"TEARDOWN")){
      msg.method=RTSP_METHOD_TEARDOWN;
    }else{
      msg.method=RTSP_METHOD_ERROR;
    }
    msg.URI=strtok(NULL," \n");
    if(msg.URI == NULL) {
      msg.rtspVerMajor = 1;
      msg.rtspVerMinor = 0;
      res = RTSP_RESULT_CLIENT_ERROR;
      return rtsp_answer(&msg, res, fd);
    }
    tok = strtok(NULL," /");//пропускаем RTSP
    if (tok == NULL || strcmp(tok,"RTSP")) {
      msg.rtspVerMajor = 1;
      msg.rtspVerMinor = 0;
      res = RTSP_RESULT_CLIENT_ERROR;
      return rtsp_answer(&msg, res, fd);
    }
    tok = strtok(NULL,"/.");
    if (tok == NULL) {
      msg.rtspVerMajor = 1;
      msg.rtspVerMinor = 0;
      res = RTSP_RESULT_CLIENT_ERROR;
      return rtsp_answer(&msg, res, fd);
    }
    msg.rtspVerMajor=atoi(tok);
    tok = strtok(NULL,". \n");
    if (tok == NULL) {
      msg.rtspVerMajor = 1;
      msg.rtspVerMinor = 0;
      res = RTSP_RESULT_CLIENT_ERROR;
      return rtsp_answer(&msg, res, fd);
    }
    msg.rtspVerMinor=atoi(tok);
    msg.cseq=0;
    msg.fieldsCount=0;

    msg.fields=(struct Field *) malloc(sizeof(struct Field));
    tok=strtok(NULL,": \n");//cтрока
    while(tok != NULL){
      //парсинг пары заголовок: значение
      msg.fields=(struct Field *) realloc((void *)msg.fields,sizeof(struct Field)*(msg.fieldsCount+=1));
      msg.fields[msg.fieldsCount-1].header=tok;
      tok=strtok(NULL,": \n");
      if(tok != NULL){
        msg.fields[msg.fieldsCount-1].value=(char *) malloc(sizeof(char)*strlen(tok));
        strcpy(msg.fields[msg.fieldsCount-1].value,tok);
      }else{
        free(buf);
        tok=NULL;
        //strcpy(wbuf,"RSTP/1.0 451 Parameter Not Understood");
        //goto Write;
        res = RTSP_RESULT_PARAM_ERROR;
        return rtsp_answer(&msg, res, fd);
      }
      //проверка на Content-Length
      if(!strcmp(msg.fields[msg.fieldsCount-1].header,"Content-Length")){
        // +2 для безопасности, для \0
        msg.content=strtok(NULL,"\0");
        free(buf);
        //tok=NULL;
        //strcpy(wbuf,"DONE");
        //goto Write;
      }
      tok=strtok(NULL,": \n");//cтрока
    }

  }
  /*
  requestSize = write(fd,wbuf,strlen(wbuf));
  if(requestSize == -1) {
    return -1;
  }
  return 0;
  */
  if(msg.method == RTSP_METHOD_ERROR) {
    res = RTSP_RESULT_CLIENT_ERROR;
  } else {
    res = RTSP_RESULT_OK;
  }
  return rtsp_answer(&msg, res, fd);
}

int rtsp_answer(RtspMessage *rtsp, enum RtspResults res, int fd) {
  char buf[BUF_SIZE];
  char *point = buf, *title;
  RtspAnswer ans;
  int ret, i, fst = 0;
  ans.code = res;
  ans.rtspVerMajor = rtsp->rtspVerMajor;
  ans.rtspVerMinor = rtsp->rtspVerMinor;
  sprintf(point,"RTSP/%d.%d ", ans.rtspVerMajor, ans.rtspVerMinor);
  point += strlen(buf);
  switch(ans.code) {
    case RTSP_RESULT_OK:
      title = "200 OK\n";
      rtsp_fields(rtsp, &ans);
      fst = 1;
      break;
    case RTSP_RESULT_PARAM_ERROR:
      title = "451 Parameter Not Understood\n";
      break;
    default:
      /*
      Не обрабатываем RTSP_RESULT_CLIENT_ERROR отдельно, так как
      ошибка 400 обозначает все остальные случаи
      */
      title = "400 Bad Request\n";
  }
  strcpy(point, title);
  point = buf + strlen(buf);
  if(fst && rtsp->fieldsCount) {
    for(i = 0;i < rtsp->fieldsCount;i++) {
      sprintf(point,"\n%s : %s", ans.fields[i].header, ans.fields[i].value);
      point = buf + strlen(buf);
    }
  }
  ret = write(fd,buf,strlen(buf));
  if(ret == -1) {
    perror("write");
    die_pos_err(errno);
  }
  if(fst && rtsp->fieldsCount) {
    free(rtsp->fields);
    free(ans.fields);
  }
  return strlen(buf);
}

int rtsp_fields(RtspMessage *rtsp, RtspAnswer *ans) {
  int i;
  ans->fieldsCount = 0;
  ans->fields = malloc(sizeof(struct Field));
  for(i = 0;i < rtsp->fieldsCount;i++) {
    /*
    TODO: Не просто копировать из запроса, а обрабатывать
    */
    ans->fieldsCount++;
    ans->fields[i].header = rtsp->fields[i].header;
    ans->fields[i].value = rtsp->fields[i].value;
    ans->fields = realloc(ans->fields,sizeof(struct Field) * ans->fieldsCount);
  }
  return i;
}
