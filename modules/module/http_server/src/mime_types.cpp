#include <string>
#include <map>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <algorithm>

#include "util.h"

using namespace std;
  
typedef map<string, string> MIME_TYPE_MAP;

MIME_TYPE_MAP g_MimeTypeMap;

char *locate_mime_end(char *line)
{
	char *p = line;
	while (*p){
		if (*p == ' ' || *p == '\t' || *p == ';' || *p == ',')
			break;
		p++;
	}
	
	return p;
}

bool add_mime_type(MIME_TYPE_MAP &MimeMap, char *line)
{
	string mime_type("");
	string extension("");
	char *start = NULL, *end = NULL;
	char *p = NULL;
	bool ret = false;
	
	p = strchr(line, '#');
	if (p) *p = 0x00;
	
	//获取mime_type
	start = line;
	p   = locate_mime_end(start);
	if (start == p)
		return ret;
		
	*p = 0x00;
	mime_type = start;
	transform(mime_type.begin(), mime_type.end(), mime_type.begin(), ::tolower);
	
	start = p + 1;
	end = start + strlen(start);
	while(start < end){
		clean_line(start);
		p = locate_mime_end(start);
		
		if (p <= start)
			break;
			
		*p = 0x00;
		extension = start;
		transform(extension.begin(), extension.end(), extension.begin(), ::tolower);
		MimeMap[extension] = mime_type;
		ret = true;
			
		start = p + 1;
	}
	
	return ret; 
}

bool load_mime_types(const char *filename)
{
	int len = 0;
	char line[1024 * 4];
	FILE *fp = NULL;
		
	if (filename == NULL){
		return false;
	}
	
	fp = fopen(filename, "r");
	if (fp == NULL){
		return false;
	}

	bzero(line, sizeof(line));
	
	while(fgets(line, sizeof(line), fp) != NULL){
		len = strlen(line);
		//1:去掉注释
		char *p = strchr(line, '#');
		if (p != NULL)
			*p = '\0';
	
		//2：去掉两头的不可见字符
		clean_line(line);
		len = strlen(line);
		if (line[0] == '#' || strlen(line) < 3)
			continue;
			
		add_mime_type(g_MimeTypeMap, line);

		bzero(line, sizeof(line));
	}
	
	fclose(fp);
	return true;
}

const char *find_mime_type(const char *fileext)
{
	static std::string default_mime_type = "application/octet-stream";
    if (fileext == NULL)
        //return NULL;
        return default_mime_type.c_str();

    string lower_ext = fileext;
    transform(lower_ext.begin(), lower_ext.end(), lower_ext.begin(), ::tolower);
	MIME_TYPE_MAP::iterator it = g_MimeTypeMap.find(lower_ext.c_str());
		
	if (it != g_MimeTypeMap.end()){
		return it->second.c_str();
	}
	
	//return NULL;//default_mime_type.c_str();
	return default_mime_type.c_str();
}


