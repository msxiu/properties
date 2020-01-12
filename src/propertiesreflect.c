#include "llqreflect.h"
#include "libproperties.h"


static int propertysection2entity(propertysection_p section, void* e, reflect_p flds){
	size_t i=0;
	reflect_p f;
	if(section && flds && e) {
		const char* value;
		while((f = (flds + i)) && f->fldname && (REFLECT_NONE != f->type) && f->entire) {
            switch(f->type){
            case REFLECT_INT:
                if(section && (value = propertysection_valueof(section,f->fldname))) {
                    reflect_setintvalue(f, e, atoi(value));
                }
                break;
            case REFLECT_UINT: 
            case REFLECT_TIME:
                if(section && (value = propertysection_valueof(section,f->fldname))) {
                    reflect_setuintvalue(f, e, atol(value)); 
                }
                break;
            case REFLECT_FLOAT: 
                if(section && (value = propertysection_valueof(section,f->fldname))) {
                    reflect_setfloatvalue(f, e, atof(value)); 
                }
                break;
            case REFLECT_STRBUF:
            case REFLECT_STRING: 
                if(section && (value = propertysection_valueof(section,f->fldname))) {
                    reflect_mallocstring(f, e, value);
                }
                break;
            default:break;
            }
			i++;
		}
	}
	return i;
}

int properties2entiry(properties_p o, void* e, reflect_p flds) {
	size_t i=0;
	reflect_p f;
	if(o && flds && e) {
		reflect_p innerflds;
		void* childentry;
		const unsigned char* v = (const unsigned char*)e;
        propertysection_p firstsection = properties_defaultsection(o);
        if(firstsection) {
            propertysection2entity(firstsection, e, flds);
        }
		while((f = (flds + i)) && f->fldname && (REFLECT_NONE != f->type) && f->entire) {
            switch(f->type){
             case REFLECT_OBJBUF:
                if(f->getreflect && (innerflds = f->getreflect()) && (firstsection = propertysection_find(o, f->fldname))) {
                    childentry = (void*)(v + f->offset);
                    propertysection2entity(firstsection, childentry, innerflds);
                }
                break;
            case REFLECT_OBJECT:
                if(f->getreflect && (innerflds = f->getreflect()) && (firstsection = propertysection_find(o, f->fldname))) {
                    if((childentry = (*((void**)(v + f->offset)) = calloc(1, innerflds->entire)))) {
                        propertysection2entity(firstsection, childentry, innerflds);
                    }
                }
                break;
            default:break;
            }
			i++;
		}
	}
	return i;
}

/*** 从配置文件中加载到实体对象中 */
int propertiesentity4file(void* entity, reflect_p flds, const char* faddr) {
   properties_p conf = NULL;
    if(entity && flds && faddr && (conf = properties_loader(faddr, 3))) {
        properties2entiry(conf, entity, flds);
        free(conf);
        return 1;
    }
    return 0;
}

