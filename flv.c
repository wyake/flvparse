#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#define MAXBUF 2048

struct KeyFrames{
	int offsize;
	int timetamp;
	struct KeyFrames* next;
};

struct KeyFrames* KeyListInit(){  
	struct KeyFrames *Keyhead;  
	Keyhead = (struct KeyFrames *)malloc(sizeof(struct KeyFrames));     
	if(Keyhead == NULL){                       
		printf("	Key head malloc error\n");  
		return NULL;
	}
	Keyhead->next = NULL;       
	return Keyhead;
}  

int KeyListInsterHead(struct KeyFrames *Keyhead,int offsize,int timetamp){  
	struct KeyFrames *p;  
	p = (struct KeyFrames *)malloc(sizeof(struct KeyFrames));  
	if(p==NULL){
		printf("	inster new node fail\n");
		return 0;
	}
	p->offsize = offsize;                        
	p->timetamp =timetamp;   	 
	p->next=Keyhead->next;
	Keyhead->next = p;   
	return 1;   
} 

int KeyListInsterTail(struct KeyFrames *Keyhead,int offsize,int timetamp){  
	struct KeyFrames *p,*phead;  
	p = (struct KeyFrames *)malloc(sizeof(struct KeyFrames));  
	if(p==NULL){
		printf("	inster new node fail\n");
		return 0;
	}
	p->offsize = offsize;                        
	p->timetamp =timetamp;  
	p->next=NULL; 
	phead=Keyhead;
	while(phead->next != NULL)
	{
		phead = phead->next;
	}
	phead->next=p;  
   
 	return 1;   
}   

void TraverseKeyList(struct KeyFrames *Keyhead){
	struct KeyFrames *p=Keyhead->next;                            
	while(p!=NULL)                                
	{
		printf("	Keyframes  offsize  %d  ,timetemp  %d\n",p->offsize,p->timetamp); 
		p = p->next;                              
	}

} 

void DeleteAll(struct KeyFrames *head){
	struct KeyFrames *p = head;
	struct KeyFrames *q;
	if (head == NULL)  
		return;
	while(p->next != NULL)   
	{
		q = p->next;
        	p->next = p->next->next;
        	free(q);
	}
	free(p);  
	printf("	delete all\n");                           
	head = NULL;                              
}

int TraverseChange_p(FILE* newfd,struct KeyFrames *Keyhead,int bytecount){
	struct KeyFrames *p=Keyhead->next;   
	char buf[MAXBUF];
	int i=0;
	unsigned char* co;
	int count=0;
	double temp;
	while(p!=NULL)                                
	{
		//printf("	add Keyframes  offsize  %d\n",p->offsize); 
		p->offsize+=bytecount;
		//printf("	add Keyframes  new offsize  %d\n",p->offsize);
		temp=(double)p->offsize;
		co=(unsigned char*)&(temp);
		buf[i+0]=0;
		buf[i+1]=co[7];
		buf[i+2]=co[6];
		buf[i+3]=co[5];
		buf[i+4]=co[4];
		buf[i+5]=co[3];
		buf[i+6]=co[2];
		buf[i+7]=co[1];
		buf[i+8]=co[0];
		//printf("	Keyframes  offsize  %d\n",p->offsize);
		p = p->next;   
		i+=9;
	}
	if((count=fwrite(buf,i,1,newfd))!=1){
		printf("add opsition write error %s\n",strerror(errno));
		return -1;
	}
	return 1;
}

int TraverseChange_t(FILE* newfd,struct KeyFrames *Keyhead,int bytecount){
	struct KeyFrames *p=Keyhead->next;   
	char buf[MAXBUF];
	int i=0;
	unsigned char* co;
	int count=0;
	double temp;
	while(p!=NULL)                                
	{
		//printf("	add Keyframes  timetamp  %d\n",p->timetamp);
		temp=(double)p->timetamp/1000;
		//printf("	add Keyframes  new timetamp  %lf\n",temp);
		co=(unsigned char*)&(temp);
		buf[i+0]=0;
		buf[i+1]=co[7];
		buf[i+2]=co[6];
		buf[i+3]=co[5];
		buf[i+4]=co[4];
		buf[i+5]=co[3];
		buf[i+6]=co[2];
		buf[i+7]=co[1];
		buf[i+8]=co[0];
		//printf("	Keyframes  offsize  %d\n",p->offsize);
		p = p->next;   
		i+=9;
	}
	if((count=fwrite(buf,i,1,newfd))!=1){
		printf("add timetamp write error %s\n",strerror(errno));
		return -1;
	}
	return 1;
}


void TimePrintf(int timetamp){
	int hour = timetamp/(3600*1000);  
        //printf("%2d:",hour);  
        timetamp-= hour * 3600*1000;  
        int minutes = timetamp/(60*1000);  
        timetamp-= minutes * 60*1000;  
        //printf("%2d:",minutes);  
        int seconds = timetamp/1000;  
        //printf("%2d:",seconds);  
        timetamp-= seconds*1000;  
        //printf("%3d\n",timetamp); //millisecond 
	printf("	%2d:%2d:%2d, %2d\n",hour,minutes,seconds,timetamp);
}

int HeadParse(FILE* fd,unsigned char buf[]){
	int datasize;
	memset(buf,0,sizeof(buf));
	fseek(fd,0,SEEK_SET);
	if(fread(buf,9,1,fd)==1){
		printf("----------------header--------------\n");
		printf("	header start offset: %ld\n",ftell(fd)-9);
		int i=0;
		for(;i<3;i++){
			printf("	header signature: %c\n",buf[i]);
		}
		printf("	header version: %d\n",buf[3]);
		if(buf[4]&1)
			printf("	header has video\n");
		else
			printf("	header don't has video\n");
		if((buf[4]>>2)&1)
			printf("	header has audio\n");
		else
			printf("	header don't has audio\n");
		i=0;
		for(;i<4;i++){
			datasize=datasize<<8|buf[5+i];
		}
		printf("	header datasize: %d\n",datasize);
		return datasize;
	}
	else{
		printf("fread header error:%s\n",strerror(errno));
		return -1;
	}
}

int ParseAudio(FILE* fd,unsigned char buf[]){
	
	int datasize=0;
	int timetamp=0;
	int streamid=0; 
	printf("\n----------------audio--------------\n");
	printf("	audio start offset: %ld\n",ftell(fd)-11);
	printf("	script type: %d\n",buf[0]);
	int i=0;
	for(;i<3;i++){
		datasize=datasize<<8|buf[i+1];
	}
	printf("	audio datasize: %d\n",datasize);

	//timetamp
	///*
	i=2;
	for(;i>=0;i--){
		timetamp=timetamp<<8|buf[i+4];
	}
	timetamp=timetamp<<8;
	printf("	audio timetamp: %d\n",timetamp);
	//*/

	//time microsecond
	timetamp=0;
	if(buf[7]!=0x00){
		timetamp=timetamp<<8|buf[7];
	}
	i=0;
	for(;i<3;i++){
		timetamp=timetamp<<8|buf[i+4];
	}
	printf("	audio timetamp: %d\n",timetamp);
	TimePrintf(timetamp);

	i=0;
	for(;i<3;i++){
		streamid=streamid<<8|buf[i+8];
	}
	printf("	audio streamID: %d\n",streamid);
	return datasize;
}

int ParseVideo(FILE* fd,unsigned char buf[],struct KeyFrames *Keyhead,int *flag,int *keycount){
	
	int datasize=0;
	int timetamp=0;
	int streamid=0; 
	int offsize=0;
	printf("\n----------------video--------------\n");
	offsize=ftell(fd)-11;
	printf("	video start offset: %d\n",offsize);
	
	printf("	video type: %d\n",buf[0]);
	int i=0;
	for(;i<3;i++){
		datasize=datasize<<8|buf[i+1];
	}
	printf("	video datasize: %d\n",datasize);
	i=2;
	for(;i>=0;i--){
		timetamp=timetamp<<8|buf[i+4];
	}
	timetamp=timetamp<<8;
	printf("	video timetamp: %d\n",timetamp);

	//time microsecond
	timetamp=0;
	if(buf[7]!=0x00){
		timetamp=timetamp<<8|buf[7];
	}
	i=0;
	for(;i<3;i++){
		timetamp=timetamp<<8|buf[i+4];
	}
	printf("	audio timetamp: %d\n",timetamp);
	TimePrintf(timetamp);


	i=0;
	for(;i<3;i++){
		streamid=streamid<<8|buf[i+8];
	}
	
	printf("	video streamID: %d\n",streamid);
	memset(buf,0,sizeof(buf));
	fread(buf,1,1,fd);
	buf[0]=buf[0]>>4;
	if(buf[0]==1){//keyfream
		(*keycount)++;
		printf("is keyframs keycount %d\n",*keycount);
		if(KeyListInsterTail(Keyhead,offsize,timetamp)==1){
			(*flag)+=1;
			printf("key inster success flag is:%d\n",*flag);
		}
	}
	return datasize-1;
}


void ReadStrictArray(FILE* fd,int arraynumber){//strict
	char buf[50];
	while(arraynumber--){
		memset(buf,0,sizeof(buf));
		fread(buf,1,1,fd);
		if(buf[0]==0x00){
			
		}
		if(buf[0]==0x01){
			
		}
	}
}


void ReadArray(FILE* fd){
	char buf[60];
	double value;
	unsigned char* co;
	while(1){
		printf("\n");
		memset(buf,0,sizeof(buf));
		fread(buf,2,1,fd);
		
		if((buf[0]==0x00)&&(buf[1]==0x00)){
			memset(buf,0,sizeof(buf));
			fread(buf,1,1,fd);
			if(buf[0]==0x09){
				printf("	}\n");
				break;
			}
		}
		
		int namelength=0;
		namelength=namelength<<8|buf[0];
		namelength=namelength<<8|buf[1];
		//printf("	script AMF2 propnamelength: %d\n",namelength);
		
		memset(buf,0,sizeof(buf));
		fread(buf,namelength,1,fd);
		buf[namelength]='\0';
		printf("	script AMF2 propname: %s\n",buf);
		
		memset(buf,0,sizeof(buf));
		fread(buf,1,1,fd);
		printf("	script AMF2 proptype: %d\n",buf[0]);
		
		if(buf[0]==0x00){//double
			memset(buf,0,sizeof(buf));
			value=0.0;
			fread(buf,8,1,fd);
			
			co = (unsigned char* )&value;
			co[0]=buf[7];
			co[1]=buf[6];
			co[2]=buf[5];
			co[3]=buf[4];
			co[4]=buf[3];
			co[5]=buf[2];
			co[6]=buf[1];
			co[7]=buf[0];
			printf("	double AMF2 propvalue: %lf\n",value);
		}
		else if(buf[0]==0x01){//bool
			memset(buf,0,sizeof(buf));
			fread(buf,1,1,fd);
			printf("	bool AMF2 propvalue: %d\n",buf[0]);
		}
		else if(buf[0]==0x02){//string
			memset(buf,0,sizeof(buf));
			fread(buf,2,1,fd);
			namelength=0;
			namelength=namelength<<8|buf[0];
			namelength=namelength<<8|buf[1];
			//printf("string length %d\n",namelength);
			memset(buf,0,sizeof(buf));
			fread(buf,namelength,1,fd);
			buf[namelength]='\0';
			printf("	string AMF2 propvalue: %s\n",buf);
		}
		else if(buf[0]==0x03){//object
			//break;
			ReadArray(fd);
		}
		else if(buf[0]==0x08){
			fread(buf,4,1,fd);
			ReadArray(fd);
		}
		else if(buf[0]==0x0a){//strict array
			memset(buf,0,sizeof(buf));
			fread(buf,4,1,fd);
			namelength=0;
			namelength=namelength<<8|buf[0];
			namelength=namelength<<8|buf[1];
			namelength=namelength<<8|buf[2];
			namelength=namelength<<8|buf[3];
			/**/
			while(namelength--){
				memset(buf,0,sizeof(buf));
				fread(buf,1,1,fd);
				if(buf[0]==0x00){
					memset(buf,0,sizeof(buf));
					value=0.0;
					fread(buf,8,1,fd);
					unsigned char* co;
					co = (unsigned char* )&value;
					co[0]=buf[7];
					co[1]=buf[6];
					co[2]=buf[5];
					co[3]=buf[4];
					co[4]=buf[3];
					co[5]=buf[2];
					co[6]=buf[1];
					co[7]=buf[0];
					printf("	double AMF2 propvalue: %lf\n",value);
				}
			}
			/**/				
		}
		else{
			printf("	type no know\n");
		}
	}
}

void Body_AMF2(FILE* fd){
	//body AMF2
	printf("\n");
	char buf[6];
	memset(buf,0,sizeof(buf));
	fread(buf,5,1,fd);
	printf("	script AMF2 type: %d\n",buf[0]);
	int arraylen=0;
	arraylen=arraylen<<8|buf[1];
	arraylen=arraylen<<8|buf[2];
	arraylen=arraylen<<8|buf[3];
	arraylen=arraylen<<8|buf[4];
	printf("	script AMF2 array number: %d\n	{\n",arraylen);
	ReadArray(fd);
}


void Body_AMF1(FILE* fd){
	//FIRST AMF
	printf("\n");
	char buf[14];
	memset(buf,0,sizeof(buf));
	fread(buf,13,1,fd);
	printf("	script AMF1 type: %d\n",buf[0]);
	//unsigned char* c = (unsigned char* )buf;
	// vnamelength=(c[1]<<8)|c[2];
	int namelength=0;
	namelength=namelength<<8|buf[1];
	namelength=namelength<<8|buf[2];
	printf("	script AMF1 length: %d\n",namelength);
	/*
	char AMPcont[10];
	i=3;
	int k=0;
	for(;i<=12;i++){
		AMPcont[k++]=buf[i];
	}
	printf("	script AMF1 cotent: %s\n",AMPcont);
	*/
	printf("	script AMF1 cotent: ");
	int i=3;
	while(i<13){
		printf("%c",buf[i]);
		i++;
	}
	printf("\n");
}

int ParseScript(FILE* fd,unsigned char buf[],int *scriptsize){
	int datasize=0,timetamp=0,streamid=0;
	int namelength;
	double value;
	printf("\n----------------script--------------\n");
	printf("	script start offset: %ld\n",ftell(fd)-11);
	printf("	script type: %d\n",buf[0]);
	int i=0;
	for(;i<3;i++){
		datasize=datasize<<8|buf[i+1];
	}
	printf("	script datasize: %d\n",datasize);
	*scriptsize=datasize;
	i=2;
	for(;i>=0;i--){
		timetamp=timetamp<<8|buf[i+4];
	}
	timetamp=timetamp<<8;
	printf("	script timetamp: %d\n",timetamp);


	//time microsecond
	timetamp=0;
	if(buf[7]!=0x00){
		timetamp=timetamp<<8|buf[7];
	}
	i=0;
	for(;i<3;i++){
		timetamp=timetamp<<8|buf[i+4];
	}
	printf("	script timetamp: %d\n",timetamp);
	TimePrintf(timetamp);

	i=0;
	for(;i<3;i++){
		streamid=streamid<<8|buf[i+8];
	}
	printf("	script streamID: %d\n",streamid);
	
	Body_AMF1(fd);
	
	Body_AMF2(fd);
	return 0;
}

void getfilename(char filepath[]){
	char *index = NULL;
    char *ret = NULL;
	char name[10];
    int i = 0;
    do{
        index = strstr(filepath + i++, "/");
        if (index != NULL)
            ret = index;
    }while(index != NULL);
	ret++;
	strcpy(name,ret);
	int length=strlen(ret);
	while(length--){
		*ret='\0';
		ret++;
	}
	strcat(filepath,"1234");
	strcat(filepath,name);
}

int AddNewContent(FILE* newfd,struct KeyFrames* Keyhead,int keycount){
	int bytecount=0;
	char objectname[20]="keyframes";
	char offname[20]="filepositions";
	char timename[20]="times";
	int len1=strlen(objectname);
	int len2=strlen(offname);
	int len3=strlen(timename);
	unsigned char* co;
	char buf[40];
	int i;
	int count=0;
	
	/*
	printf("please input new object name:\n");
	scanf("%s",objectname);
	*/
	bytecount+=len1+len2+len3;
	bytecount+=keycount*18;
	bytecount+=20;
	printf("bytecount %d\n",bytecount);
	
	co=(unsigned char*)&len1;
	memset(buf,0,sizeof(buf));
	buf[0]=co[1];
	buf[1]=co[0];
	i=0;
	for(;i<len1;i++)
		buf[i+2]=objectname[i];
	buf[len1+2]=0x03;
	if((count=fwrite(buf,len1+3,1,newfd))!=1){
		printf("objec\"keyframes\" inster error %s\n",strerror(errno));
		return -1;
	}
	
	memset(buf,0,sizeof(buf));
	co=(unsigned char*)&len2;
	buf[0]=co[1];
	buf[1]=co[0];
	i=0;
	for(;i<len2;i++)
		buf[i+2]=offname[i];
	buf[len2+2]=0x0a;
	len2+=3;
	co=(unsigned char*)&keycount;
	buf[len2++]=co[3];
	buf[len2++]=co[2];
	buf[len2++]=co[1];
	buf[len2++]=co[0];
	if((count=fwrite(buf,len2,1,newfd))!=1){
		printf("stritarray \"filepositions\" inster error %s\n",strerror(errno));
		return -1;
	}
	
	if(TraverseChange_p(newfd,Keyhead,bytecount)<0){
		return -1;
	}
	
	memset(buf,0,sizeof(buf));
	co=(unsigned char*)&len3;
	buf[0]=co[1];
	buf[1]=co[0];
	i=0;
	for(;i<len3;i++)
		buf[i+2]=timename[i];
	buf[len3+2]=10;
	len3+=3;
	co=(unsigned char*)&keycount;
	buf[len3++]=co[3];
	buf[len3++]=co[2];
	buf[len3++]=co[1];
	buf[len3++]=co[0];
	if((count=fwrite(buf,len3,1,newfd))!=1){
		printf("stritarray \"times\" inster error %s\n",strerror(errno));
		return -1;
	}
	
	if(TraverseChange_t(newfd,Keyhead,bytecount)<0){
		return -1;
	}
	memset(buf,0,sizeof(buf));
	buf[0]=0x00;
	buf[1]=0x00;
	buf[2]=0x09;
	fwrite(buf,3,1,newfd);
	return 1;
	
}

int ChangeMetadata(FILE* fd,char filepath[],struct KeyFrames* Keyhead,int scriptsize,int keycount){
	//create new file 
	char newfilepath[60];
	strcpy(newfilepath,filepath);
	getfilename(newfilepath);
	FILE* newfd;
	char readbuf[MAXBUF];
	int count=0;
	unsigned char* co;
	
	int addscriptsize;
	char objectname[20]="keyframes";
	char offname[20]="filepositions";
	char timename[20]="times";
	int len1=strlen(objectname);
	int len2=strlen(offname);
	int len3=strlen(timename);
	addscriptsize+=len1+len2+len3;
	addscriptsize+=keycount*18;
	addscriptsize+=20;
	
	
	if((newfd=fopen(newfilepath,"ab+"))==NULL){
		printf("newfilepath: %s open error %s\n",newfilepath,strerror(errno));
		return -1;
	}
	printf("	new file path is %s\n",newfilepath);
	
	
	struct stat filebuf;
	if(stat(filepath,&filebuf)){
		printf("get ole file stat error: %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	int filelen=filebuf.st_size;
	//copy old file to new file
	fseek(fd,0,SEEK_SET);
	if((count=fread(readbuf,14,1,fd))!=1){
		printf("copy fread 38 error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	if((count=fwrite(readbuf,14,1,newfd))!=1){
		printf("copy fwrite 38 error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	//change metadata datasize
	addscriptsize+=scriptsize;
	memset(readbuf,0,sizeof(readbuf));
	fread(readbuf,4,1,fd);
	// int arraylen=0;
	memset(readbuf,0,sizeof(readbuf));
	co=(unsigned char* )&addscriptsize;
	// co[3]=readbuf[0];
	// co[2]=readbuf[1];
	// co[1]=readbuf[2];
	// co[0]=readbuf[3];
	// memcpy(&arraylen,readbuf,4);
	// printf("arraylen %d\n",arraylen);
	// arraylen+=1;
	// memset(readbuf,0,sizeof(readbuf));
	//memcpy(readbuf,&arraylen,4);
	readbuf[0]=co[3];
	readbuf[1]=co[2];
	readbuf[2]=co[1];
	readbuf[3]=co[0];
	fwrite(readbuf,4,1,newfd);
	
	if((count=fread(readbuf,21,1,fd))!=1){
		printf("copy fread 38 error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	if((count=fwrite(readbuf,21,1,newfd))!=1){
		printf("copy fwrite 38 error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	
	
	
	//modify the size of arraysize
	memset(readbuf,0,sizeof(readbuf));
	fread(readbuf,4,1,fd);
	int arraylen=0;
	co=(unsigned char* )&arraylen;
	co[3]=readbuf[0];
	co[2]=readbuf[1];
	co[1]=readbuf[2];
	co[0]=readbuf[3];
	//memcpy(&arraylen,readbuf,4);
	printf("arraylen %d\n",arraylen);
	arraylen+=1;
	memset(readbuf,0,sizeof(readbuf));
	//memcpy(readbuf,&arraylen,4);
	readbuf[0]=co[3];
	readbuf[1]=co[2];
	readbuf[2]=co[1];
	readbuf[3]=co[0];
	fwrite(readbuf,4,1,newfd);
	
	
	//copy original array content to new file
	scriptsize-=21;
	printf("scriptsize %d\n",scriptsize);
	if((count=fread(readbuf,scriptsize,1,fd))!=1){
		printf("copy fread scriptsize error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	if((count=fwrite(readbuf,scriptsize,1,newfd))!=1){
		printf("copy fwrite scriptsize error %s\n",strerror(errno));
		fclose(newfd);
		return -1;
	}
	
	if(AddNewContent(newfd,Keyhead,keycount)<0){
		fclose(newfd);
		return -1;
	}
	
	///*
	filelen-=(38+4+scriptsize);
	//printf("new filelen %d\n",filelen);
	int lastlen=filelen%MAXBUF;
	//printf("lastlen %d\n",lastlen);
	//int cishu=0;
	while(1){
		if((count=fread(readbuf,MAXBUF,1,fd))==1){
			fwrite(readbuf,MAXBUF,1,newfd);
			//cishu++;
		}
		else{
			//printf("zuihouyikuai\n");
			fread(readbuf,lastlen,1,fd);
			fwrite(readbuf,lastlen,1,newfd);
			break;
		}
		
	}
	//printf("cishu %d\n",cishu);
	//*/
	fclose(newfd);
	return 1;
}

int main(){
	FILE* fd;
	char filepath[25]="/home/book/flv/test2.FLV";
	unsigned char buf[16];
	int i=0;
	int offcount,datasize=0,scriptsize;	
	struct KeyFrames *Keyhead;
	int flag=0,keycount=0;
	Keyhead=KeyListInit();

	if((fd=fopen(filepath,"rb"))==NULL){
		printf("fopen error %s\n",strerror(errno));
		exit(1);
	}
	else{
		if((datasize=HeadParse(fd,buf))==-1){
			exit(1);
		}
		datasize=0;
		while(1){
			offcount=datasize+4;
			fseek(fd,offcount,SEEK_CUR);
			if(fread(buf,11,1,fd)==1){
				datasize=0;
				if(buf[0]==0x08){//parse  audio
					datasize=ParseAudio(fd,buf);
				}
				else if(buf[0]==0x09){//parse  video
				//maybe has keyfream
					datasize=ParseVideo(fd,buf,Keyhead,&flag,&keycount);
				}
				else{//parse  script
					datasize=ParseScript(fd,buf,&scriptsize);
					
					//break;
				}
			}
			else{
				printf("\n----------------flv parse complete----------------\n");
				break;
			}
		}
	}
	
	//print Keyhead
	if(Keyhead==NULL)
		exit(1);
	printf("	flag %d,  keycount %d\n",flag,keycount);
	if(flag==keycount){
		printf("\n----------------begin change metadata----------------\n");
		TraverseKeyList(Keyhead);
		printf("scriptsize %d\n",scriptsize);
		///*
		if(ChangeMetadata(fd,filepath,Keyhead,scriptsize,keycount)<0){
			printf("Change Metadata fail\n");
			DeleteAll(Keyhead);
			fclose(fd);
			exit(1);
		}
		//*/
		DeleteAll(Keyhead);
		
	}
	else{
		printf("	inster Keyhead case error\n");
		DeleteAll(Keyhead);
	}
	
	fclose(fd);
	exit(0);
}