/****************************************************************
 *
 * Program: vspm2lipsia
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2005, <lipsia@cbs.mpg.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id: vspm2lipsia.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/

#include <viaio/Vlib.h>
#include <viaio/VImage.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>


#define LEN 10000    /* max line length */
#define N   2000     /* max number of onsets per line */
#define M   50       /* max number of different events */
#define K   120      /* max len of a token */

extern int VStringToken(char *, char *, int, int);
extern void getLipsiaVersion(char*,size_t);

int
main(int argc, char *argv[]) {
    static VFloat tr = 0;
    static VString filename = "";
    static VOptionDescRec  options[] = {
        {"out", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL, "Output file"},
        {"tr", VFloatRepn, 1, (VPointer) &tr, VRequiredOpt, NULL, "Repetition time in seconds"}
    };
    FILE *in_file, *fp;
    char buf[LEN], token[K];
    float onset, duration, height;
    float onset_array[M][N], onset_dim[M], duration_array[M][N], param_array[M][N];
    int j, n, id, nid = 0, pid = 0;
    char prg_name[100];
	char ver[100];
	getLipsiaVersion(ver, sizeof(ver));
	sprintf(prg_name, "vspm2lipsia V%s", ver);
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, &in_file, NULL);
    duration = 1;
    height   = 1;
    for(j = 0; j < M; j++)
        onset_dim[j] = 0;
    /*
    ** parse input file, get onsets
    */
    id = 1;
    while(!feof(in_file)) {
        memset(buf, 0, LEN);
        fgets(buf, LEN, in_file);
        if(strlen(buf) < 2)
            continue;
        if(buf[0] == '#' || buf[0] == '%')
            continue;
        n = 0;
        VStringToken(buf, token, n, 16);
        if(strcmp(token, "onset:") == 0) {
            n = 1;
            while(VStringToken(buf, token, n, K)) {
                sscanf(token, "%f", &onset);
                onset *= tr;
                onset_array[id][n] = onset;
                n++;
                if(n >= N)
                    VError(" too many onsets");
            }
            onset_dim[id] = n;
            id++;
        }
    }
    nid = id;
    rewind(in_file);
    /*
    ** get durations
    */
    id = 1;
    while(!feof(in_file)) {
        memset(buf, 0, LEN);
        fgets(buf, LEN, in_file);
        if(strlen(buf) < 2)
            continue;
        if(buf[0] == '#' || buf[0] == '%')
            continue;
        for(j = 0; j < N; j++)
            duration_array[id][j] = 1;
        n = 0;
        VStringToken(buf, token, n, 16);
        if(strcmp(token, "onset:") == 0)
            id++;
        if(strcmp(token, "param:") == 0)
            continue;
        if(strcmp(token, "duration:") == 0) {
            n = 1;
            while(VStringToken(buf, token, n, 16)) {
                sscanf(token, "%f", &duration);
                duration *= tr;
                duration_array[id - 1][n] = duration;
                n++;
            }
        }
    }
    rewind(in_file);
    /*
    ** get params
    */
    id = 1;
    while(!feof(in_file)) {
        memset(buf, 0, LEN);
        fgets(buf, LEN, in_file);
        if(strlen(buf) < 2)
            continue;
        if(buf[0] == '#' || buf[0] == '%')
            continue;
        for(j = 0; j < N; j++)
            param_array[id][j] = 0;
        n = 0;
        VStringToken(buf, token, n, 16);
        if(strcmp(token, "onset:") == 0)
            id++;
        if(strcmp(token, "duration:") == 0)
            continue;
        if(strcmp(token, "param:") == 0) {
            param_array[id - 1][0] = 1;
            n = 1;
            while(VStringToken(buf, token, n, 16)) {
                sscanf(token, "%f", &height);
                param_array[id - 1][n] = height;
                n++;
            }
        }
    }
    fclose(in_file);
    fp = fopen(filename, "w");
    if(!fp)
        VError(" error opening output file %s", filename);
    pid = nid;
    for(id = 1; id < nid; id++) {
        height = 1;
        for(j = 1; j < onset_dim[id]; j++) {
            fprintf(fp, " %3d  %10.2f  %10.2f  %10.2f\n",
                    id, onset_array[id][j], duration_array[id][j], height);
        }
        fprintf(fp, "\n");
        if(param_array[id][0] > 0) {
            for(j = 1; j < onset_dim[id]; j++) {
                fprintf(fp, " %3d  %10.2f  %10.2f  %10.2f\n",
                        pid, onset_array[id][j], duration_array[id][j], param_array[id][j]);
            }
            pid++;
            fprintf(fp, "\n");
        }
    }
    fclose(fp);
    return 0;
}







