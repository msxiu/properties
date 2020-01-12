#include "llqvarymem.h"
#include "llqfixedbuffer.h"
#include "libproperties.h"


/** --判断字符是否为空字符 */
static INLINE int properties_emptyof(short c) {
	return ((c == 0 || c == 32 || c == '\v' || c == '\t' || c == '\r' || c == '\n') ? 1 : 0);
}
/** 在流中查找字符出现的位置
 * @param ptr:		内存区首地址
 * @param size:		内存区长度
 * @param offset:	查找位置偏移
 * @param c:		查找的字符
*/
static INLINE int properties_charof(const void* ptr, uint32_t size, uint32_t offset, unsigned char c) {
	unsigned int i = 0, ln;
	if(ptr && size && offset < size) {
		const unsigned char* s = ((const unsigned char*)ptr) + offset;
		ln = (size - offset);
		while(i < ln){
			if(*(s + i) == c) return ((int)i);
			 i++;
		}
	}
	return -1;
}
/** 计算字符串(左侧)首部空格长度, @return 返回首部空格字符个数 
 * @param ptr:	数据开始地址
 * @param sln:	数据长度
*/
static INLINE int properties_emptystart(const void* ptr, int sln) {
	int i = 0;
	char ch;
	const unsigned char *s;
	if((s = (const unsigned char*)ptr) && sln) {
		while((i <= sln) && (ch = *(s+i))) {
			if(!properties_emptyof(ch)) break;
			i++;
		}
	}
	return i;
}
/** 计算字符串(右侧)尾部空格长度, *@return 返回尾部空格字符个数 
 * @param ptr:		数据开始地址
 * @param sln:		数据长度
*/
static INLINE int properties_emptyend(const void* ptr, int sln) {
	int i = 0;
	char ch;
	const unsigned char *s;
	if((s = (const unsigned char*)ptr) && sln) {
		while((i <= sln) && (ch = *(s+(sln - i - 1)))) {
			if(!properties_emptyof(ch))  break;
			i++;
		}
	}
	return i;
}

/** 从addr缓存中读取一行,根据设置移除首尾空格, @return 返回下一行位置  
 * @param s:		查找行的数据开始地址
 * @param sln:		数据长度
 * @param start:	接收有效数据开始位置
 * @param len:		接收有效数据长度
 * @param trim:		是否清空头尾空格(0:不清除,1:清除头部,2:清除尾部,3:头尾都清除)
*/
static INLINE int properties_lineof(const unsigned char* s, uint32_t sln, uint32_t* start, uint32_t* len, int trim) {
	unsigned int i = 0, kln;
	unsigned char ch = 0;
	if(s && sln) {
		*start=*len=0;
		while((i < sln) && (ch = *(s + i))) {//计算行长度
			i++;
			if (ch == '\r' || ch == '\n') break;// || ch == '\r'
		}
		if(i > 1) {
			*len = (i - ((ch == '\r' || ch == '\n') ? 1 : 0));//行的长度
			if((trim & 1) && (kln = properties_emptyend(s, *len))) {//清除尾部空格
				(*len) -= kln;
			}
			if((trim & 2) && (kln = properties_emptystart(s, *len))) {//清除首部空格
				(*start) += kln;
				(*len) -= kln;
			}
		}
		return i;
	}
	return 0;
}

/**从缓存中加载ini配置, return 返回解析到的参数个数 
 * @param faddr:	文件地址
 * @param trim:		是否清空头尾空格(0:不清除,1:清除头部,2:清除尾部,3:头尾都清除) */
properties_p properties_loadbody(const char* buf, int bln, int trim) {
	int  ptln = 0, pos =0, qps = 0, qpn = 0, qln=0;
	unsigned int pstart=0, plen=0;
	const char *cptr;
    
    offsetptr_t property = OFFSETPTR_EMPTY;//整个配置数据区
    offsetptr_t section = OFFSETPTR_EMPTY;//配置段
	if(buf && bln > 0) {//读取文件到缓存区中
		offsetptr_writeint32(&property, 0);
        while((ptln = properties_lineof((unsigned char*)(buf + pos), bln - pos, &pstart, &plen, 1))) {
			const char *kstart = (const char*)(buf + pos + pstart);
			pos += ptln;
			if(!plen) continue;//空行
			if(*kstart == '#') continue;//注示行

			if(*kstart == '[') {//段标记的改变
                if(section.offset>0) {
                    //fprintf(stdout, "section:'%s' was writed!\n", (char*)section.ptr);
					offsetptr_writeint32(&property, sizeof(uint32_t) + section.offset);
                    offsetptr_write(&property, section.ptr, section.offset);
                    offsetptr_reclaim(&section); 
                }
				cptr = kstart+1;
				if(-1 != (qpn = properties_charof(cptr, plen, 0, ']'))) {
                    offsetptr_write(&section, cptr, qpn);//写入section名称
                    offsetptr_writeint8(&section, 0);//写入结整符
					qln = qpn;
				}
				continue;
			}
			if(-1 == (qps = properties_charof(kstart, plen, 0, '='))) continue;//等号的位置
			cptr = kstart;
			qln = qps;
			if((trim & 2) && (qpn = properties_emptyend(cptr, qln))) {
				qln -= qpn;//去掉key尾部空格
			}
			qps++;
			if(0 == section.offset) {
				offsetptr_writeint8(&section, 0);//表示无名section
			}
			//fprintf(stdout, "key:%.*s\n", qln, cptr);
            offsetptr_write(&section, cptr, qln);//写入key
            offsetptr_writeint8(&section, 0);
			cptr = (char*)(kstart + qps);
			qln = (plen - qps);
			if((trim & 1) && (qpn= properties_emptystart(cptr, qln))) {//去掉val首部空格
				cptr = (char*)(kstart + qps + qpn);
				qln = (plen - qps - qpn);
			}
			//fprintf(stdout, "value:%.*s\n", qln, cptr);
            offsetptr_write(&section, cptr, qln);//写入value
            offsetptr_writeint8(&section, 0);
		}
        if(section.offset>0) {
			//fprintf(stdout, "section:'%s' was writed!\n", (char*)section.ptr);
            offsetptr_writeint32(&property, sizeof(uint32_t) + section.offset);
            offsetptr_write(&property, section.ptr, section.offset);
            offsetptr_reclaim(&section); 
        }
        *((int*)(property.ptr)) = property.offset - sizeof(uint32_t);//总长度不包含在缓存长度中
	}
	return (properties_p)property.ptr;
}


/**从缓存中加载ini配置, return 返回解析到的参数个数 
 * @param faddr:	文件地址
 * @param trim:		是否清空头尾空格(0:不清除,1:清除头部,2:清除尾部,3:头尾都清除) */
properties_p properties_loader(const char* faddr, int trim) {
	properties_p res = NULL;//反回对象
	FILE *fp;
	char* buffer = NULL;
	int rlen = 0, flen=0, tln;
	if(faddr && (fp = fopen(faddr, "rb"))) {//成功打开文件
		fseek(fp, 0L, SEEK_END);
		flen = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		if((buffer = (char*)calloc(1, flen + 10))) {
			while(rlen < flen){
				tln = fread(buffer + rlen, 1, (flen - rlen), fp);
				rlen += tln;
			}
			res = properties_loadbody(buffer, rlen, trim);
			free(buffer);
		}
		fclose(fp);
	}
	return res;
}
