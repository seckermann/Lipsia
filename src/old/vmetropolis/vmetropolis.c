/****************************************************************
 *
 * Program: vmetropolis
 *
 * Copyright (C) Max Planck Institute
 * for Human Cognitive and Brain Sciences, Leipzig
 *
 * Author Gabriele Lohmann, 2004, <lipsia@cbs.mpg.de>
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
 * $Id: vmetropolis.c 3181 2008-04-01 15:19:44Z karstenm $
 *
 *****************************************************************/


#include <viaio/Vlib.h>
#include <viaio/mu.h>
#include <viaio/option.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define N 1000   /* max number of trials      */
#define M 50     /* max number of event types */

extern char *getLipsiaVersion();

void
print_mat(float mat[M][M], int ntypes, FILE *fp) {
    int i, j;
    for(i = 0; i < ntypes; i++) {
        for(j = 0; j < ntypes; j++) {
            fprintf(fp, " %8.3f", mat[i][j]);
        }
        fprintf(fp, "\n");
    }
    fprintf(fp, "\n");
}



int
main(int argc, char *argv[]) {
    static VString filename = "";
    static VString out_filename = "";
    static VLong  seed    = 42;
    static VShort ntypes  = 3;
    static VLong  maxiter = -1;
    static VOptionDescRec  options[] = {
        {"in", VStringRepn, 1, (VPointer) &filename, VRequiredOpt, NULL, "input file"},
        {"report", VStringRepn, 1, (VPointer) &out_filename, VRequiredOpt, NULL, "output report file"},
        {"ntypes", VShortRepn, 1, (VPointer) &ntypes, VRequiredOpt, NULL, "number of event types"},
        {"seed", VLongRepn, 1, (VPointer) &seed, VOptionalOpt, NULL, "seed"},
        {"iter", VLongRepn, 1, (VPointer) &maxiter, VOptionalOpt, NULL, "max number of iterations"}
    };
    FILE *fp, *fp_out;
    float u, mat[M][M], cooc[M][M];
    int trial[N], trial_tmp[N], list[N];
    int ntrials;
    int histo[M];
    float sum, nx, mx, dist, mindist;
    float d, p, r, t, t0;
    int i, j = 0, k = 0;
    long iter = 0, i0 = 0;
    int j1, k1, j0, k0;
    unsigned char buf[256];
    size_t n = 256;
    VBoolean verbose = TRUE;
    char prg_name[50];
    sprintf(prg_name, "vmetropolis V%s", getLipsiaVersion());
    fprintf(stderr, "%s\n", prg_name);
    VParseFilterCmd(VNumber(options), options, argc, argv, NULL, NULL);
    if(ntypes >= M)
        VError(" max number(%d) of event types exceeded", M);
    if(maxiter < 0) {
        maxiter = VRepnMaxValue(VLongRepn);
        fprintf(stderr, " max number of iterations: %ld\n", maxiter);
    }
    /*
    ** get input data
    */
    memset(cooc, 0, sizeof(float)*M * M);
    memset(mat, 0, sizeof(float)*M * M);
    memset(histo, 0, sizeof(int)*N);
    fp = fopen(filename, "r");
    if(!fp)
        VError(" error opening file %s", filename);
    i = 0;
    ntrials = 0;
    while(fgets(buf, n, fp) && i < ntypes) {
        if(strlen(buf) < 2)
            continue;
        if(buf[0] == '#')
            continue;
        sscanf(buf, "%d", &j);
        histo[i] = j;
        ntrials += j;
        i++;
    }
    if(ntrials >= N)
        VError(" max number(%d) of trials exceeded", N);
    /* get model co-occurrence matrix */
    i = 0;
    while(fgets(buf, n, fp) && i < ntypes) {
        if(strlen(buf) < 2)
            continue;
        if(buf[0] == '#')
            continue;
        u = atof(strtok(buf, " "));
        mat[i][0] = u;
        for(j = 1; j < ntypes; j++) {
            u = atof(strtok(NULL, " "));
            mat[i][j] = u;
        }
        i++;
    }
    fclose(fp);
    fp_out = fopen(out_filename, "w");
    if(!fp_out)
        VError(" error opening report file %s", out_filename);
    if(verbose) {
        fprintf(stderr, "\n user-specified transition probabilities:\n");
        fprintf(fp_out, "\n user-specified transition probabilities:\n");
        print_mat(mat, ntypes, stderr);
        print_mat(mat, ntypes, fp_out);
    }
    /* normalize matrix */
    for(i = 0; i < ntypes; i++) {
        sum = nx = 0;
        for(j = 0; j < ntypes; j++) {
            if(mat[i][j] >= 0)
                sum += mat[i][j];
            else
                nx++;
        }
        if(sum < 0.0001 && nx == 0)
            continue;
        if(nx == ntypes) {
            for(j = 0; j < ntypes; j++)
                mat[i][j] = -1;
            continue;
        }
        mx = (float)(ntypes - nx) / (float)(ntypes);
        for(j = 0; j < ntypes; j++) {
            if(mat[i][j] >= 0) {
                if(sum > 0)
                    mat[i][j] /= sum;
                mat[i][j] *= mx;
            } else
                mat[i][j] = -1;
        }
    }
    if(verbose) {
        fprintf(stderr, "\n normalized user-specified transition probabilities:\n");
        fprintf(fp_out, "\n normalized user-specified transition probabilities:\n");
        print_mat(mat, ntypes, stderr);
        print_mat(mat, ntypes, fp_out);
    }
    k = 0;
    for(i = 0; i < ntypes; i++) {
        for(j = 0; j < histo[i]; j++)
            trial_tmp[k++] = i;
    }
    /*
    ** select an initial random permutation
    */
    for(i = 0; i < N; i++)
        list[i] = -1;
    srand((unsigned int) seed);
    for(i = 0; i < ntrials; i++) {
nochmal:
        j = (int)((float)ntrials * rand() / (RAND_MAX + 1.0));
        for(k = 0; k < i; k++)
            if(list[k] == j)
                goto nochmal;
        trial[i] = trial_tmp[j];
        list[i] = j;
    }
    if(verbose) {
        fprintf(stderr, " initial random permutation:\n");
        fprintf(fp_out, " initial random permutation:\n");
        for(i = 0; i < ntrials; i++) {
            fprintf(stderr, " %d", trial[i]);
            fprintf(fp_out, " %d", trial[i]);
        }
        fprintf(fp_out, "\n\n");
        fprintf(stderr, "\n\n");
    }
    /* compute new co-occurrences */
    memset(cooc, 0, sizeof(float)*M * M);
    for(j = 0; j < ntrials - 1; j++) {
        j1 = trial[j];
        k1 = trial[j + 1];
        cooc[j1][k1]++;
    }
    for(i = 0; i < ntypes; i++) {
        sum = 0;
        for(j = 0; j < ntypes; j++)
            sum += cooc[i][j];
        for(j = 0; j < ntypes; j++)
            cooc[i][j] /= sum;
    }
    /*
    ** start metropolis
    */
    mindist = VRepnMaxValue(VFloatRepn);
    dist    = 1.0;
    t0      = 3.0;
    fprintf(stderr, "\n iterations...\n");
    iter = 0;
    while(mindist > 0.0001 && iter < maxiter) {
        j0 = (int)((float)ntrials * rand() / (RAND_MAX + 1.0));
        k0 = (int)((float)ntrials * rand() / (RAND_MAX + 1.0));
        if(j0 == k0)
            continue;
        /* switch positions */
        j1 = trial[j0];
        k1 = trial[k0];
        trial[j0] = k1;
        trial[k0] = j1;
        /* compute new co-occurrences */
        memset(cooc, 0, sizeof(float)*M * M);
        for(j = 0; j < ntrials - 1; j++) {
            j1 = trial[j];
            k1 = trial[j + 1];
            cooc[j1][k1]++;
        }
        for(i = 0; i < ntypes; i++) {
            sum = 0;
            for(j = 0; j < ntypes; j++)
                sum += cooc[i][j];
            for(j = 0; j < ntypes; j++)
                cooc[i][j] /= sum;
        }
        /* evaluate quality of new co-occurrences */
        dist = 0;
        for(j = 0; j < ntypes; j++) {
            for(k = 0; k < ntypes; k++) {
                if(mat[j][k] >= 0) {
                    d = cooc[j][k] - mat[j][k];
                    dist += d * d;
                }
            }
        }
        /* save current optimum */
        if(dist < mindist) {
            mindist = dist;
            i0 = iter;
            for(i = 0; i < ntrials; i++) {
                trial_tmp[i] = trial[i];
            }
        }
        /* simulated annealing */
        if(dist < 0.001)
            goto ende;
        u = (float)(iter + 1) / (float)(maxiter + 1);
        t = -t0 * log((double) u);
        p = 1.0 / (1.0 + exp(-(double)(dist / t)));
        r = (float)rand() / (float)RAND_MAX;
        /* if too bad, reverse */
        if(p > r) {
            j1 = trial[j0];
            k1 = trial[k0];
            trial[j0] = k1;
            trial[k0] = j1;
        }
        if(iter % 2500000 == 0 && maxiter > 5000000 && verbose)
            fprintf(stderr, " %10ld  %10.6f\r", iter, mindist);
        iter++;
    }
    if(verbose)
        fprintf(stderr, "\n");
    /*
    ** output
    */
ende:
    /* get current optimum */
    if(dist < mindist) {
        mindist = dist;
        for(i = 0; i < ntrials; i++) {
            trial[i] = trial_tmp[i];
        }
    }
    memset(cooc, 0, sizeof(float)*M * M);
    for(j = 0; j < ntrials - 1; j++) {
        j1 = trial[j];
        k1 = trial[j + 1];
        cooc[j1][k1]++;
    }
    for(i = 0; i < ntypes; i++) {
        sum = 0;
        for(j = 0; j < ntypes; j++)
            sum += cooc[i][j];
        for(j = 0; j < ntypes; j++)
            cooc[i][j] /= sum;
    }
    if(verbose) {
        fprintf(stderr, "\n iteration: %ld,  dist: %f\n", i0, mindist);
        fprintf(stderr, " resulting transition probabilities:\n");
        print_mat(cooc, ntypes, stderr);
        fprintf(stderr, " resulting sequence:\n");
        for(i = 0; i < ntrials; i++)
            fprintf(stderr, " %d", trial[i]);
        fprintf(stderr, "\n\n");
        fprintf(fp_out, "\n iteration: %ld,  dist: %f\n", i0, mindist);
        fprintf(fp_out, " resulting transition probabilities:\n");
        print_mat(cooc, ntypes, fp_out);
        fprintf(fp_out, " resulting sequence:\n");
        for(i = 0; i < ntrials; i++)
            fprintf(fp_out, " %d", trial[i]);
        fprintf(fp_out, "\n\n");
        fclose(fp_out);
    }
    fprintf(stderr, "%s: done.\n", argv[0]);
    return 0;
}
