#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

struct KeyFrames{
	int offsize;
	int timetamp;
	struct KeyFrames* next;
};

struct KeyFrames* KeyListInit(){  
	struct KeyFrames *Keyhead;  
	Keyhead = (struct KeyFrames *)malloc(sizeof(struct KeyFrames));     
	if(Keyhead == NULL){                       
		printf("Key head malloc error\n");  
		return NULL;
	}
	Keyhead->next = NULL;       
	return Keyhead;
}  

int KeyListInster(struct KeyFrames *Keyhead,int offsize,int timetamp)  
{  
    struct KeyFrames *p;  
    p = (struct KeyFrames *)malloc(sizeof(struct KeyFrames));  
	if(p==NULL){
		printf("inster new node error\n");
		return 0;
	}
    p->offsize = offsize;                        
    p->timetamp =timetamp;   	 
	p->next=Keyhead->next;
    Keyhead->next = p;   
    return 1;   
}   

void TraverseKeyList(struct KeyFrames *Keyhead)
{
	struct KeyFrames *p=Keyhead->next;                            
	while(p!=NULL)                                
	{
		printf("Keyframes  offsize  %d  ,",p->offsize); 
		printf("timetemp  %d",p->timetamp);		
		p = p->next; 
		printf("\n");                               
	}

} 

void DeleteAll(struct KeyFrames *head)
{
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
    head = NULL;                              
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

int ParseVideo(FILE* fd,unsigned char buf[]){
	
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
		//(*keycount)++;
		printf("is keyframs keycount\n");
		// if(KeyListInster(Keyhead,offsize,timetamp)==1){
			// (*flag)+=1;
			// printf("key inster success flag is:%d\n",*flag);
		// }
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

int ParseScript(FILE* fd,unsigned char buf[]){
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

int main(){
	FILE* fd;
	char filepath[25]="/home/book/flv/01.flv";
	unsigned char buf[16];
	int i=0;
	int offcount;	
	int datasize=0;
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
					datasize=ParseVideo(fd,buf);
				}
				else{//parse  script
					datasize=ParseScript(fd,buf);
					//break;
				}
			}
			else{
				printf("	flv parse complete\n");
				break;
			}
		}
	}
	
	fclose(fd);
	exit(0);
}