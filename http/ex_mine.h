/**
 * Copyright @2019 Exserver All Rights Reserved.
 */

#ifndef EX_MINE_H
#define EX_MINE_H

static char *MIMES[] = {
	"text/html", /*0*/
	"text/css",  /*1*/
	"text/xml",  /*2*/
	"image/gif", /*3*/
	"image/jpeg",/*4*/
	"application/javascript", /*5*/
	"application/json",       /*6*/
	"text/plain",             /*7*/
	"image/png",              /*8*/
	"image/tiff",             /*9*/
	"image/webp",             /*10*/
	"image/x-icon",           /*11*/
	"image/x-jng",            /*12*/
	"image/x-ms-bmp",         /*13*/
	"video/mp4",              /*14*/
	"video/mpeg",             /*15*/
	"video/webm",             /*16*/
	"audio/mpeg",             /*17*/
	"application/octet-stream", /*18*/
	"application/zip",          /*19*/
	"application/x-rar-compressed"  /*20*/
};

char *ex_get_mine_type(char *f);


#endif  /* EX_MINE_H */
