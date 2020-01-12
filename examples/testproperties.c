#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "libproperties.h"
#include "llqreflect.h"

typedef struct author {
    int id;
    char name[64];
    char summary[256];
}author_t, * author_p;

typedef struct objentity {
    int id;
    int type;
    int group;
    author_t author;
    char title[255];
    char content[255];
    time_t time;
} objentity_t, * objentity_p;

static reflect_p author_reflectof() {
    static reflect_t reflds[] = {
        REFLECT_INIT(REFLECT_INT, author_t, id),
        REFLECT_INIT(REFLECT_STRBUF, author_t, name),
        REFLECT_INIT(REFLECT_STRBUF, author_t, summary),
        REFLECT_EMPTY
    };
    return reflds;
}

reflect_p objentity_reflectof() {
    static reflect_t reflds[] = {
        REFLECT_INIT(REFLECT_INT, objentity_t, id),
        REFLECT_INIT(REFLECT_INT, objentity_t, type),
        REFLECT_INIT(REFLECT_INT, objentity_t, group),
        REFLECT_CALLBACK(REFLECT_OBJBUF, objentity_t, author, author_reflectof),
        REFLECT_INIT(REFLECT_STRBUF, objentity_t, title),
        REFLECT_INIT(REFLECT_STRBUF, objentity_t, content),
        REFLECT_INIT(REFLECT_UINT, objentity_t, time),
        REFLECT_EMPTY
    };
    return reflds;
}
void objentity_show(objentity_p o, FILE* out){
    fprintf(out, "----------- id:%d ------------------------------------------------\n", o->id);
    fprintf(out, "type:\t%d\n", o->type);
    fprintf(out, "group:\t%d\n", o->group);
    fprintf(out, "time:\t%d\n", (int)o->time);
    fprintf(out, "title:\t%s\n", o->title);
    fprintf(out, "content:\t%s\n", o->content);
    fprintf(out, "-----------author.id:%d ------------------------------------------------\n", o->author.id);
    fprintf(out, "author.name:\t%s\n", o->author.name);
    fprintf(out, "author.summary:\t%s\n", o->author.summary);
}





int main(int argc, char* argv[]) {
    propertysection_p section, sections[128];
    int i, count = 0;
    size_t offset =0;
    const char *scename, *key, *value, *faddr = "./httpengine-response.properties";
    if(argc > 1) { faddr = argv[1]; }
    properties_p conf = properties_loader(faddr, 3);
    objentity_t objitem;
    if(conf) {
        count = propertysection_loader(conf, 128, sections);
        for(i=0; i<count; i++) {
            section = sections[i];
            fprintf(stdout, "-- section:'%s' -----------------------------\n", section->buffer);
            offset = 0;
            while(propertysection_nexof(section, &offset, &key, &value)) {
                fprintf(stdout, "%s:'%s'\n", key, value);
            }
        }
        offset = 0;
        fprintf(stdout, "\n%s\n", "-----------------------------------------------------------------");
        while(properties_nexof(conf, &offset, &scename, &key, &value)) {
            fprintf(stdout, "%s=>%s:'%s'\n", scename, key, value);
        }

        free(conf);
    }
    // if((conf = properties_loader("./config-objectity.properties", 3))) {
    //     reflect_p flds = objentity_reflectof();
    //     memset(&objitem, 0, sizeof(objentity_t));
    //     properties2entiry(conf, &objitem, flds);
    //     objentity_show(&objitem, stdout);
    //     free(conf);
    // }
    if(propertiesentity4file(&objitem, objentity_reflectof(), "./config-objectity.properties")) {
        objentity_show(&objitem, stdout);
    }
}

