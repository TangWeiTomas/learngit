#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <errno.h>
#include "SimpleDBMgr.h"
#include "logUtils.h"

#include "interface_devicelist.h"
#include "interface_scenelist.h"
#include "interface_deviceStatelist.h"

#define MAX_DB_FILENAMR_LEN 255

void SimpleDBMgr_Init(void)
{
    char dbFilename[MAX_DB_FILENAMR_LEN]={0};
    
#ifdef OPENWRT_TEST
    char* where = "/etc/config/dbData";
#else
    char* where = "./dbData";
#endif

    if(mkdir(where,S_IRWXU|S_IRWXG|S_IRWXO) == 0)
    {
        log_err("mkdir success: %s\n",strerror(errno));
    }
    else
    {
        log_err("mkdir failed: %s\n",strerror(errno));
    }

    sprintf(dbFilename, "%s/devicelistfile.dat",where);
    log_debug("zllMain: device DB file name %s\n", dbFilename);
    devListInitDatabase(dbFilename);

    sprintf(dbFilename, "%s/devStatelistfile.dat",where);
    log_debug("zllMain: devState DB file name %s\n", dbFilename);
    devStateListInitDatabase(dbFilename);
	
    sprintf(dbFilename, "%s/scenelistfile.dat", where);
    log_debug("zllMain: scene DB file name %s\n", dbFilename);
    sceneListInitDatabase(dbFilename);

	sprintf(dbFilename, "%s/eventlistfile.dat" ,where);
	log_debug("zllMain: Event DB file name %s\n", dbFilename);
	eventListInitDatabase(dbFilename);

	sprintf(dbFilename, "%s/timetasklistfile.dat" ,where);
	log_debug("zllMain: timetask DB file name %s\n", dbFilename);
	timeTaskListInitDatabase(dbFilename);
}

void SimpleDBMgr_release(void)
{
	devListrelaseDatabase();
	devStateListrelaseDatabase();
	sceneListrelaseDatabase();
	eventListrelaseDatabase();
	timeTaskListrelaseDatabase();
}
