#include "llqvarymem.h"
#include "llqfixedbuffer.h"
#include "libproperties.h"


static int propertysection_positionAt(properties_p o, uint32_t offset, propertysection_p* section, uint32_t* secstart) {
    uint32_t pos = 0;
    propertysection_p item = NULL;
    while(pos < o->size) {
        item = (propertysection_p)(((unsigned char*)(o->buffer)) + pos);
        if(pos <= offset && (pos + item->size) > offset) {
            *section = item;
            *secstart = pos;
            return 1;
        }
        pos += item->size;
    }
    return 0;
}

/** 通过段名查找段 */
propertysection_p propertysection_find(properties_p o, const char *section) {
    uint32_t pos = 0;
    propertysection_p item = NULL;
    while(pos < o->size) {
        item = (propertysection_p)(((unsigned char*)(o->buffer)) + pos);
        if(!strcmp(section, item->buffer)) return item;
        pos += item->size;
    }
    return NULL;
}
/** 加载全部段 */
int propertysection_loader(properties_p o, int argc, propertysection_p argv[]) {
    int count = 0;
    uint32_t pos = 0;
    propertysection_p item = NULL;
    while(pos < o->size) {
        item = (propertysection_p)(((unsigned char*)(o->buffer)) + pos);
        argv[count++] = item;
        pos += item->size;
    }
    return count;
}

int propertysection_nexof(propertysection_p o, size_t* offset, const char** key, const char** value) {
    size_t size = o->size - 4;//除去字节数描述长度
    if(0 == (*offset)) {
        fixedbuffer_stringof(o->buffer, size, offset);
    }
    *key    = fixedbuffer_stringof(o->buffer, size, offset);
    *value  = fixedbuffer_stringof(o->buffer, size, offset);
    return ((*key) && (*value));
}
const char* propertysection_valueof(propertysection_p o, const char* pkey) {
    size_t offset = 0, size = o->size - 4;//除去字节数描述长度
    const char* key, *value;
    fixedbuffer_stringof(o->buffer, size, &offset);//读取section名称
    while(offset < size) { 
        key    = fixedbuffer_stringof(o->buffer, size, &offset);
        value  = fixedbuffer_stringof(o->buffer, size, &offset);
        if(key && value && (0 == strcmp(key, pkey))) return value;
     }
     return NULL;
}

propertysection_p properties_defaultsection(properties_p o) {
    if(0 == o->buffer[4]) {
        return ((propertysection_p)o->buffer);
    }
    return NULL;
}

/**用于在while循环中查找是否有下一个记录*/
int properties_nexof(properties_p o, size_t* offset, const char** secname, const char** key, const char** value) {
	unsigned int pos = (*offset), secstart = 0;
    size_t secpos;
	if(o && (pos < o->size)) {
        propertysection_p section = NULL;
        if(!propertysection_positionAt(o, pos, &section, &secstart)) return 0;
        secpos = pos - secstart;
        if(propertysection_nexof(section, &secpos, key, value)) {
            *offset     = secpos + secstart;
            *secname    = section->buffer;
            return 1;
        }
        pos = secstart + section->size;//跳到下一个section开始位置
        if(pos < o->size) {
            secstart    = pos;
            secpos      = 0;
            section     = (propertysection_p)(((unsigned char*)(o->buffer)) + pos);
            if(propertysection_nexof(section, &secpos, key, value)) {
                *offset     = secpos + secstart;
                *secname    = section->buffer;
                return 1;
            }
        }
	}
	return 0;
}
const char* properties_valueof(properties_p o, const char* psection, const char* pkey){
    size_t offset = 0;
    const char *res = NULL, *section = NULL, *key = NULL, *value = NULL;
    while(properties_nexof(o, &offset, &section, &key, &value)) {
        if(!strcasecmp(pkey, key)) {
			if(!strcasecmp(psection, section))return value;
			if(NULL == section) res = value;//找到根的key值
		}
    }
	return res;
}


