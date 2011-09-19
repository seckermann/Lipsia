/* From the standard C libaray: */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRINGIFY(x) #x
#define EXPAND(x) STRINGIFY(x)

/*
void getLipsiaVersion(char*,size_t) {
    char url[] = "$HeadURL: https://svnserv.cbs.mpg.de/svn/gr_lipsia/lipsia-2.0/trunk/src/old/lib_lipsia/GetVersion.c $";
    char *ver = (char *)VMalloc(sizeof(char) * 20);
    char *pch;
    // check if this version was checked out from the trunk 
    if(strstr(url, "/trunk/")) {
        sprintf(ver, "#TRUNK#");
    }
  //   check if this version comes from a '/tags/' subdir 
    else {
        if((pch = strstr(url, "/tags/"))) {
            // in 'tags' lipsia should reside in a directory similar to 'lipsia-X.X.X/'.
             X.X.X is the version string we are interested in.
            pch = strstr(url, "lipsia-");
            pch = strtok(pch, "-");
            pch = strtok(NULL, "/");
            if(!pch)
                sprintf(ver, "0.0.0");
            else
                strcpy(ver, pch);
        }
  //      /* obviously, this version comes from outer space. Hence, there is no number. 
        else
            sprintf(ver, "0.0.0");
    }
    return ver;
}
*/
void getLipsiaVersion(char *ver, size_t length) {
#ifdef LIPSIA_RCS_REVISION
	snprintf(ver, length, "%s.%s.%s [%s]",EXPAND(_LIPSIA_VERSION_MAJOR), EXPAND(_LIPSIA_VERSION_MINOR), EXPAND(_LIPSIA_VERSION_PATCH), EXPAND(_LIPSIA_RCS_REVISION) );
#else
	snprintf(ver, length, "%s.%s.%s",EXPAND(_LIPSIA_VERSION_MAJOR), EXPAND(_LIPSIA_VERSION_MINOR), EXPAND(_LIPSIA_VERSION_PATCH) );
#endif 
}

