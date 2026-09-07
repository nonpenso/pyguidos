/***********************************************************************
Author(s): Pierre Soille
Copyright (C) 2000-2020 European Union (Joint Research Centre)

This file is part of miallib.

miallib is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

miallib is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with miallib.  If not, see <https://www.gnu.org/licenses/>.
***********************************************************************/



#ifndef _MIALLIB_LABEL_H
#define _MIALLIB_LABEL_H       1

#include "mialtypes.h"


/* label.c */
extern ERROR_TYPE label(IMAGE *im1, IMAGE *im2, int ox, int oy, int oz);
extern ERROR_TYPE labelpixngb(IMAGE *im1, IMAGE *im2, int ox, int oy, int oz);
extern ERROR_TYPE labelplat(IMAGE *im1, IMAGE *im2, int ox, int oy, int oz);
extern ERROR_TYPE seededlabelplat(IMAGE *im1, IMAGE *im2, IMAGE *im3, int ox, int oy, int oz);
extern ERROR_TYPE seededplat(IMAGE *im1, IMAGE *im2, IMAGE *im3, int ox, int oy, int oz);
extern ERROR_TYPE labelpix(IMAGE *im);
extern ERROR_TYPE resolveLabels(IMAGE *imlbl, IMAGE *imlut, IMAGE *imlutback, int graph);
extern ERROR_TYPE gorder(IMAGE *lbl, IMAGE *g, int n);

/* mmlabel.c */
extern IMAGE *erodelabel(IMAGE *im, int graph);

/* propagate.c */
extern ERROR_TYPE propagate(IMAGE *lbl, IMAGE *dst,  IMAGE **imap, int nc, int graph);

/* setreg.c */
extern ERROR_TYPE set_regions(IMAGE *ilbl, IMAGE *ival, int indic);
extern ERROR_TYPE setregionsgraph(IMAGE *ilbl, IMAGE *ival, int indic, int graph);
extern ERROR_TYPE tessel_surface(IMAGE *im);
extern ERROR_TYPE relabel(IMAGE *ilbl1, IMAGE *ilbl2, IMAGE *iarea2);

/* setreglut.c  */
extern IMAGE *region_lut(IMAGE *ilbl, int graph, int type, int param1, int param2);
extern IMAGE *region_lut_seq(IMAGE *ilbl, int graph, int type);
extern IMAGE *region_im_lut(IMAGE *ilbl, IMAGE *im, int graph, int type, float aval);
extern IMAGE *contortion_lut(IMAGE *ilbl, int graph);
extern IMAGE *moments_lut_to_ellipse_lut(IMAGE **impq);

/* alphacc.c */
extern IMAGE *alphacc(IMAGE *dissx, IMAGE *dissy, int alpha);


/* labelvertex.c  */
extern IMAGE *labelvertex(IMAGE *im, int alpha, int graph);
extern IMAGE *vertexseparation(IMAGE *im, int graph, int type);
extern IMAGE *labelvertexconnectedness(IMAGE *im, int alpha, int graph, int deg);

/* labelccfastrim.c */
extern IMAGE *labelcc(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int rg, int rl);

/* labelccms.c */
extern IMAGE *labelccms(IMAGE **imap, int nc, IMAGE *imse, int ox, int oy, int oz, int r1, int r2);

/* labelccmi.c */
extern IMAGE *labelccmi(IMAGE *im, IMAGE *immi, IMAGE *imse, int ox, int oy, int oz, int rg, int rl);

/* labelci.c */
extern IMAGE *labelci(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int rl);

/* labelcims.c:  */
extern IMAGE *labelcims(IMAGE **imap, int nc, IMAGE *imse, int ox, int oy, int oz, int rl);

/* labelccdissim.c */
extern IMAGE *labelccdissim(IMAGE *im, IMAGE *imh, IMAGE *imv, int rg, int rl);

/* labelccvar.c */
extern IMAGE *labelccvar(IMAGE *im, IMAGE *imse, int ox, int oy, int oz, int rg, int rl, double varmax);

/* labelccmsdissim.c: */
extern IMAGE *labelccmsdissim(IMAGE **imap, int nc, IMAGE *imh, IMAGE *imv, int rg, int rl);

/* newlabelcc.c */
extern IMAGE *labelccattr(IMAGE *im, int graph, int rg, int rl);

/* alphatree.c */
extern IMAGE **alphatree(IMAGE *dissx, IMAGE *dissy, int alphamax);
extern IMAGE *alphatreeincattr(IMAGE **atree, IMAGE **attr0cc, int type);
extern IMAGE *alphatreetoCCs(IMAGE **atree, IMAGE *imblbl, IMAGE *flaglut, int rule);
extern IMAGE *alphatreenextlevel(IMAGE **atree, IMAGE *crtprtlabel, int alpha);
extern IMAGE *alphatreepersistencelut(IMAGE **atree);


/* alphatreetoCCs.c */
/* none exposed, used internally in alphatree.c */

/* edgeweight.c */
extern IMAGE *edgeweight(IMAGE *im, int dir, int type);

/* dbscan.c */
extern IMAGE *dissim(IMAGE **imap, int nc, IMAGE *mask, int type);
extern IMAGE *dbscan(IMAGE *dissim, double eps, int MinPts);

/* outeredge.c */
extern IMAGE *outeredgelut(IMAGE *ilbl, IMAGE *iedgelbl);
extern IMAGE *outeredge(IMAGE *ilbl, int graph);
extern IMAGE *outercontour(IMAGE *ilbl, int graph);



#endif /* miallib_label.h */
