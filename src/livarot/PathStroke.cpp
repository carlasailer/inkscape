/*
 *  PathStroke.cpp
 *  nlivarot
 *
 *  Created by fred on Tue Jun 17 2003.
 *
 */

#include "Path.h"
#include "Shape.h"
#include <libnr/nr-point.h>
#include <libnr/nr-point-fns.h>
#include <libnr/nr-matrix.h>
#include <libnr/nr-rotate.h>
#include <libnr/nr-rotate-ops.h>
#include <math.h>

/*
 * stroking polylines into a Shape instance
 * grunt work.
 * if the goal is to raster the stroke, polyline stroke->polygon->uncrossed polygon->raster is grossly
 * inefficient (but reuse the intersector, so that's what a lazy programmer like me does). the correct way would be
 * to set up a supersampled buffer, raster each polyline stroke's part (one part per segment in the polyline, plus 
 * each join) because these are all convex polygons, then transform in alpha values
 */

// until i find something better
NR::Point StrokeNormalize(const NR::Point value) {
    double length = L2(value); 
    if ( length < 0.0000001 ) { 
		return NR::Point(0, 0);
    } else { 
        return value/length; 
    }
}

// faster version if length is known
NR::Point StrokeNormalize(const NR::Point value, double length) {
    if ( length < 0.0000001 ) { 
        return NR::Point(0, 0);
    } else { 
        return value/length; 
    }
}

void Path::Stroke(Shape *dest, bool doClose, double width, JoinType join,
                  ButtType butt, double miter, bool justAdd)
{
    if (dest == NULL) {
        return;
    }
    
    if (justAdd == false) {
        dest->Reset(3 * pts.size(), 3 * pts.size());
    }
    
    dest->MakeBackData(false);

    int lastM = 0;
    while (lastM < int(pts.size())) {

        int lastP = lastM + 1;
        while (lastP < int(pts.size()) // select one subpath
               && (pts[lastP].isMoveTo == polyline_lineto
                   || pts[lastP].isMoveTo == polyline_forced))
        {
	    lastP++;
        }

        if ( lastP > lastM+1 ) {
            NR::Point sbStart = pts[lastM].p;
            NR::Point sbEnd = pts[lastP - 1].p;
            if ( NR::LInfty(sbEnd-sbStart) < 0.00001 ) {       // why close lines that shouldn't be closed?
                // ah I see, because close is defined here for
                // a whole path and should be defined per subpath.
                // debut==fin => ferme (on devrait garder un element pour les close(), mais tant pis)
                DoStroke(lastM, lastP - lastM, dest, true, width, join, butt, miter, true);
            } else {
                DoStroke(lastM, lastP - lastM, dest, doClose, width, join, butt, miter, true);
            }
        } else if (butt == butt_round) {       // special case: zero length round butt is a circle
            int last[2] = { -1, -1 };
            NR::Point dir;
            dir[0] = 1;
            dir[1] = 0;
            NR::Point pos = pts[lastM].p;
            DoButt(dest, width, butt, pos, dir, last[RIGHT], last[LEFT]);
            int end[2];
            dir = -dir;
            DoButt(dest, width, butt, pos, dir, end[LEFT], end[RIGHT]);
            dest->AddEdge (end[LEFT], last[LEFT]);
            dest->AddEdge (last[RIGHT], end[RIGHT]);
        }
        lastM = lastP;
    }
}

void Path::DoStroke(int off, int N, Shape *dest, bool doClose, double width, JoinType join,
                    ButtType butt, double miter, bool justAdd)
{
    if (N <= 1) {
        return;
    }
  
    NR::Point prevP, nextP;
    int prevI, nextI;
    int upTo;
  
    int curI = 0;
    NR::Point curP = pts[off].p;
  
    if (doClose) {

        prevI = N - 1;
        while (prevI > 0) {
            prevP = pts[off + prevI].p;
            NR::Point diff = curP - prevP;
            double dist = dot(diff, diff);
            if (dist > 0.001) {
                break;
            }
            prevI--;
        }
        if (prevI <= 0) {
            return;
        }
        upTo = prevI;

    } else {
        
        prevP = curP;
        prevI = curI;
        upTo = N - 1;
    }
    
    {
        nextI = 1;
        while (nextI <= upTo) {
            nextP = pts[off + nextI].p;
            NR::Point diff = curP - nextP;
            double dist = dot(diff, diff);
            if (dist > 0.0) { // more tolerance for the first distance, to give the cap the right direction
                break;
            }
            nextI++;
        }
        if (nextI > upTo) {
            if (butt == butt_round) {  // special case: zero length round butt is a circle
                int last[2] = { -1, -1 };
                NR::Point dir;
                dir[0] = 1;
                dir[1] = 0;
                DoButt(dest, width, butt, curP, dir, last[RIGHT], last[LEFT]);
                int end[2];
                dir = -dir;
                DoButt(dest, width, butt, curP, dir, end[LEFT], end[RIGHT]);
                dest->AddEdge (end[LEFT], last[LEFT]);
                dest->AddEdge (last[RIGHT], end[RIGHT]);
            }
            return;
        }
    }

    int start[2] = { -1, -1 };
    int last[2] = { -1, -1 };
    NR::Point prevD = curP - prevP;
    NR::Point nextD = nextP - curP;
    double prevLe = NR::L2(prevD);
    double nextLe = NR::L2(nextD);
    prevD = StrokeNormalize(prevD, prevLe);
    nextD = StrokeNormalize(nextD, nextLe);
    
    if (doClose) {
        DoJoin(dest,  width, join, curP, prevD, nextD, miter, prevLe, nextLe, start, last);
    } else {
        nextD = -nextD;
        DoButt(dest,  width, butt, curP, nextD, last[RIGHT], last[LEFT]);
        nextD = -nextD;
    }
    
    do {
        prevP = curP;
        prevI = curI;
        curP = nextP;
        curI = nextI;
        prevD = nextD;
        prevLe = nextLe;
        nextI++;
        while (nextI <= upTo) {
            nextP = pts[off + nextI].p;
            NR::Point diff = curP - nextP;
            double dist = dot(diff, diff);
            if (dist > 0.001 || (nextI == upTo && dist > 0.0)) { // more tolerance for the last distance too, for the right cap direction
                break;
            }
            nextI++;
        }
        if (nextI > upTo) {
            break;
        }
    
        nextD = nextP - curP;
        nextLe = NR::L2(nextD);
        nextD = StrokeNormalize(nextD, nextLe);
        int nSt[2] = { -1, -1 };
        int nEn[2] = { -1, -1 };
        DoJoin(dest, width, join, curP, prevD, nextD, miter, prevLe, nextLe, nSt, nEn);
        dest->AddEdge(nSt[LEFT], last[LEFT]);
        last[LEFT] = nEn[LEFT];
        dest->AddEdge(last[RIGHT], nSt[RIGHT]);
        last[RIGHT] = nEn[RIGHT];
    } while (nextI <= upTo);
    
    if (doClose) {
		/*		prevP=curP;
				prevI=curI;
				curP=nextP;
				curI=nextI;
				prevD=nextD;*/
        nextP = pts[off].p;

        nextD = nextP - curP;
        nextLe = NR::L2(nextD);
        nextD = StrokeNormalize(nextD, nextLe);
        int nSt[2] = { -1, -1 };
        int nEn[2] = { -1, -1 };
        DoJoin(dest,  width, join, curP, prevD, nextD, miter, prevLe, nextLe, nSt, nEn);
        dest->AddEdge (nSt[LEFT], last[LEFT]);
        last[LEFT] = nEn[LEFT];
        dest->AddEdge (last[RIGHT], nSt[RIGHT]);
        last[RIGHT] = nEn[RIGHT];
        
        dest->AddEdge (start[LEFT], last[LEFT]);
        dest->AddEdge (last[RIGHT], start[RIGHT]);
        
    } else {

        int end[2];
        DoButt (dest,  width, butt, curP, prevD, end[LEFT], end[RIGHT]);
        dest->AddEdge (end[LEFT], last[LEFT]);
        dest->AddEdge (last[RIGHT], end[RIGHT]);
    }
}


void Path::DoButt(Shape *dest, double width, ButtType butt, NR::Point pos, NR::Point dir,
                  int &leftNo, int &rightNo)
{
	NR::Point nor;
	nor = dir.ccw();
  
	if (butt == butt_square)
	{
		NR::Point x;
		x = pos + width * dir + width * nor;
		int bleftNo = dest->AddPoint (x);
		x = pos + width * dir - width * nor;
		int brightNo = dest->AddPoint (x);
		x = pos + width * nor;
		leftNo = dest->AddPoint (x);
		x = pos - width * nor;
		rightNo = dest->AddPoint (x);
		dest->AddEdge (rightNo, brightNo);
		dest->AddEdge (brightNo, bleftNo);
		dest->AddEdge (bleftNo, leftNo);
	}
	else if (butt == butt_pointy)
	{
		leftNo = dest->AddPoint (pos + width * nor);
		rightNo = dest->AddPoint (pos - width * nor);
		int mid = dest->AddPoint (pos + width * dir);
		dest->AddEdge (rightNo, mid);
		dest->AddEdge (mid, leftNo);
	}
	else if (butt == butt_round)
	{
		const NR::Point sx = pos + width * nor;
		const NR::Point ex = pos - width * nor;
		leftNo = dest->AddPoint (sx);
		rightNo = dest->AddPoint (ex);
    
        RecRound (dest, rightNo, leftNo, ex, sx, -nor, nor, pos, width);
	}
	else
	{
		leftNo = dest->AddPoint (pos + width * nor);
		rightNo = dest->AddPoint (pos - width * nor);
		dest->AddEdge (rightNo, leftNo);
	}
}


void Path::DoJoin (Shape *dest, double width, JoinType join, NR::Point pos, NR::Point prev,
                   NR::Point next, double miter, double prevL, double nextL,
                   int *stNo, int *enNo)
{
    NR::Point pnor = prev.ccw();
    NR::Point nnor = next.ccw();
    double angSi = cross(next, prev);

    /* FIXED: this special case caused bug 1028953 */
    if (angSi > -0.0001 && angSi < 0.0001) {
        double angCo = dot (prev, next);
        if (angCo > 0.9999) {
            // tout droit
            stNo[LEFT] = enNo[LEFT] = dest->AddPoint(pos + width * pnor);
            stNo[RIGHT] = enNo[RIGHT] = dest->AddPoint(pos - width * pnor);
        } else {
            // demi-tour
            const NR::Point sx = pos + width * pnor;
            const NR::Point ex = pos - width * pnor;
            stNo[LEFT] = enNo[RIGHT] = dest->AddPoint (sx);
            stNo[RIGHT] = enNo[LEFT] = dest->AddPoint (ex);
            if (join == join_round) {
                RecRound (dest, enNo[LEFT], stNo[LEFT], ex, sx, -pnor, pnor, pos, width);
            dest->AddEdge(stNo[RIGHT], enNo[RIGHT]);
            } else {
                dest->AddEdge(enNo[LEFT], stNo[LEFT]);
                dest->AddEdge(stNo[RIGHT], enNo[RIGHT]);	// two times because both are crossing each other
            }
        }
        return;
    }
    
    if (angSi < 0) {
        int midNo = dest->AddPoint(pos);
        stNo[LEFT] = dest->AddPoint(pos + width * pnor);
        enNo[LEFT] = dest->AddPoint(pos + width * nnor);
        dest->AddEdge(enNo[LEFT], midNo);
        dest->AddEdge(midNo, stNo[LEFT]);
        
        if (join == join_pointy) {

            stNo[RIGHT] = dest->AddPoint(pos - width * pnor);
            enNo[RIGHT] = dest->AddPoint(pos - width * nnor);
      
            const NR::Point biss = StrokeNormalize(prev - next);
            double c2 = dot(biss, nnor);
            double l = width / c2;
            double emiter = width * c2;
            if (emiter < miter) {
                emiter = miter;
            }
            
            if (fabs(l) < miter) {
                int const n = dest->AddPoint(pos - l * biss);
                dest->AddEdge(stNo[RIGHT], n);
                dest->AddEdge(n, enNo[RIGHT]);
            } else {
                dest->AddEdge(stNo[RIGHT], enNo[RIGHT]);
            }
            
        } else if (join == join_round) {
            NR::Point sx = pos - width * pnor;
            stNo[RIGHT] = dest->AddPoint(sx);
            NR::Point ex = pos - width * nnor;
            enNo[RIGHT] = dest->AddPoint(ex);
      
            RecRound(dest, stNo[RIGHT], enNo[RIGHT], 
                    sx, ex, -pnor, -nnor, pos, width);
            
        } else {
            stNo[RIGHT] = dest->AddPoint(pos - width * pnor);
            enNo[RIGHT] = dest->AddPoint(pos - width * nnor);
            dest->AddEdge(stNo[RIGHT], enNo[RIGHT]);
        }
        
    } else {
        
        int midNo = dest->AddPoint(pos);
        stNo[RIGHT] = dest->AddPoint(pos - width * pnor);
        enNo[RIGHT] = dest->AddPoint(pos - width * nnor);
        dest->AddEdge(stNo[RIGHT], midNo);
        dest->AddEdge(midNo, enNo[RIGHT]);
        
        if (join == join_pointy) {
            
            stNo[LEFT] = dest->AddPoint(pos + width * pnor);
            enNo[LEFT] = dest->AddPoint(pos + width * nnor);
      
            const NR::Point biss = StrokeNormalize(next - prev);
            double c2 = dot(biss, nnor);
            double l = width / c2;
            double emiter = width * c2;
            if (emiter < miter) {
                emiter = miter;
            }
            if ( fabs(l) < miter) {
                int const n = dest->AddPoint (pos + l * biss);
                dest->AddEdge (enNo[LEFT], n);
                dest->AddEdge (n, stNo[LEFT]);
            }
            else
            {
                dest->AddEdge (enNo[LEFT], stNo[LEFT]);
            }
            
        } else if (join == join_round) {

            NR::Point sx = pos + width * pnor;
            stNo[LEFT] = dest->AddPoint(sx);
            NR::Point ex = pos + width * nnor;
            enNo[LEFT] = dest->AddPoint(ex);
      
            RecRound(dest, enNo[LEFT], stNo[LEFT], 
                    ex, sx, nnor, pnor, pos, width);
            
        } else {
            stNo[LEFT] = dest->AddPoint(pos + width * pnor);
            enNo[LEFT] = dest->AddPoint(pos + width * nnor);
            dest->AddEdge(enNo[LEFT], stNo[LEFT]);
        }
    }
}

    void
Path::DoLeftJoin (Shape * dest, double width, JoinType join, NR::Point pos,
                  NR::Point prev, NR::Point next, double miter, double prevL, double nextL,
                  int &leftStNo, int &leftEnNo,int pathID,int pieceID,double tID)
{
	NR::Point pnor=prev.ccw();
	NR::Point nnor=next.ccw();
	double angSi = cross (next, prev);
	if (angSi > -0.0001 && angSi < 0.0001)
	{
		double angCo = dot (prev, next);
		if (angCo > 0.9999)
		{
			// tout droit
			leftEnNo = leftStNo = dest->AddPoint (pos + width * pnor);
		}
		else
		{
			// demi-tour
			leftStNo = dest->AddPoint (pos + width * pnor);
			leftEnNo = dest->AddPoint (pos - width * pnor);
			int nEdge=dest->AddEdge (leftEnNo, leftStNo);
			if ( dest->hasBackData() ) {
				dest->ebData[nEdge].pathID=pathID;
				dest->ebData[nEdge].pieceID=pieceID;
				dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
			}
		}
		return;
	}
	if (angSi < 0)
	{
		/*		NR::Point     biss;
				biss.x=next.x-prev.x;
				biss.y=next.y-prev.y;
				double   c2=cross(biss,next);
				double   l=width/c2;
				double		projn=l*(dot(biss,next));
				double		projp=-l*(dot(biss,prev));
				if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
				double   x,y;
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				leftEnNo=leftStNo=dest->AddPoint(x,y);
				} else {*/
		leftStNo = dest->AddPoint (pos + width * pnor);
		leftEnNo = dest->AddPoint (pos + width * nnor);
		int midNo = dest->AddPoint (pos);
		int nEdge=dest->AddEdge (leftEnNo, midNo);
		if ( dest->hasBackData() ) {
			dest->ebData[nEdge].pathID=pathID;
			dest->ebData[nEdge].pieceID=pieceID;
			dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
		}
		nEdge=dest->AddEdge (midNo, leftStNo);
		if ( dest->hasBackData() ) {
			dest->ebData[nEdge].pathID=pathID;
			dest->ebData[nEdge].pieceID=pieceID;
			dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
		}
		//              }
	}
	else
	{
		if (join == join_pointy)
		{
			leftStNo = dest->AddPoint (pos + width * pnor);
			leftEnNo = dest->AddPoint (pos + width * nnor);
      
			const NR::Point biss = StrokeNormalize (pnor + nnor);
			double c2 = dot (biss, nnor);
			double l = width / c2;
			double emiter = width * c2;
			if (emiter < miter)
				emiter = miter;
			if (l <= emiter)
			{
				int nleftStNo = dest->AddPoint (pos + l * biss);
				int nEdge=dest->AddEdge (leftEnNo, nleftStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (nleftStNo, leftStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
			}
			else
			{
				double s2 = cross (biss, nnor);
				double dec = (l - emiter) * c2 / s2;
				const NR::Point tbiss=biss.ccw();
        
				int nleftStNo = dest->AddPoint (pos + emiter * biss + dec * tbiss);
				int nleftEnNo = dest->AddPoint (pos + emiter * biss - dec * tbiss);
				int nEdge=dest->AddEdge (nleftEnNo, nleftStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (leftEnNo, nleftEnNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (nleftStNo, leftStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
			}
		}
		else if (join == join_round)
		{
			const NR::Point sx = pos + width * pnor;
			leftStNo = dest->AddPoint (sx);
			const NR::Point ex = pos + width * nnor;
			leftEnNo = dest->AddPoint (ex);
      
            RecRound(dest, leftEnNo, leftStNo, 
                    sx, ex, pnor, nnor ,pos, width);
        
		}
		else
		{
			leftStNo = dest->AddPoint (pos + width * pnor);
			leftEnNo = dest->AddPoint (pos + width * nnor);
			int nEdge=dest->AddEdge (leftEnNo, leftStNo);
			if ( dest->hasBackData() ) {
				dest->ebData[nEdge].pathID=pathID;
				dest->ebData[nEdge].pieceID=pieceID;
				dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
			}
		}
	}
}
    void
Path::DoRightJoin (Shape * dest, double width, JoinType join, NR::Point pos,
                   NR::Point prev, NR::Point next, double miter, double prevL,
                   double nextL, int &rightStNo, int &rightEnNo,int pathID,int pieceID,double tID)
{
	const NR::Point pnor=prev.ccw();
	const NR::Point nnor=next.ccw();
	double angSi = cross (next,prev);
	if (angSi > -0.0001 && angSi < 0.0001)
	{
		double angCo = dot (prev, next);
		if (angCo > 0.9999)
		{
			// tout droit
			rightEnNo = rightStNo = dest->AddPoint (pos - width*pnor);
		}
		else
		{
			// demi-tour
			rightEnNo = dest->AddPoint (pos + width*pnor);
			rightStNo = dest->AddPoint (pos - width*pnor);
			int nEdge=dest->AddEdge (rightStNo, rightEnNo);
			if ( dest->hasBackData() ) {
				dest->ebData[nEdge].pathID=pathID;
				dest->ebData[nEdge].pieceID=pieceID;
				dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
			}
		}
		return;
	}
	if (angSi < 0)
	{
		if (join == join_pointy)
		{
			rightStNo = dest->AddPoint (pos - width*pnor);
			rightEnNo = dest->AddPoint (pos - width*nnor);
      
			const NR::Point biss = StrokeNormalize (pnor + nnor);
			double c2 = dot (biss, nnor);
			double l = width / c2;
			double emiter = width * c2;
			if (emiter < miter)
				emiter = miter;
			if (l <= emiter)
			{
				int nrightStNo = dest->AddPoint (pos - l * biss);
				int nEdge=dest->AddEdge (rightStNo, nrightStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (nrightStNo, rightEnNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
			}
			else
			{
				double s2 = cross (biss, nnor);
				double dec = (l - emiter) * c2 / s2;
				const NR::Point tbiss=biss.ccw();
        
				int nrightStNo = dest->AddPoint (pos - emiter*biss - dec*tbiss);
				int nrightEnNo = dest->AddPoint (pos - emiter*biss + dec*tbiss);
				int nEdge=dest->AddEdge (rightStNo, nrightStNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (nrightStNo, nrightEnNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
				nEdge=dest->AddEdge (nrightEnNo, rightEnNo);
				if ( dest->hasBackData() ) {
					dest->ebData[nEdge].pathID=pathID;
					dest->ebData[nEdge].pieceID=pieceID;
					dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
				}
			}
		}
		else if (join == join_round)
		{
			const NR::Point sx = pos - width * pnor;
			rightStNo = dest->AddPoint (sx);
			const NR::Point ex = pos - width * nnor;
			rightEnNo = dest->AddPoint (ex);
      
            RecRound(dest, rightStNo, rightEnNo, 
                    sx, ex, -pnor, -nnor ,pos, width);
		}
		else
		{
			rightStNo = dest->AddPoint (pos - width * pnor);
			rightEnNo = dest->AddPoint (pos - width * nnor);
			int nEdge=dest->AddEdge (rightStNo, rightEnNo);
			if ( dest->hasBackData() ) {
				dest->ebData[nEdge].pathID=pathID;
				dest->ebData[nEdge].pieceID=pieceID;
				dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
			}
		}
	}
	else
	{
		/*		NR::Point     biss;
				biss=next.x-prev.x;
				biss.y=next.y-prev.y;
				double   c2=cross(next,biss);
				double   l=width/c2;
				double		projn=l*(dot(biss,next));
				double		projp=-l*(dot(biss,prev));
				if ( projp <= 0.5*prevL && projn <= 0.5*nextL ) {
				double   x,y;
				x=pos.x+l*biss.x;
				y=pos.y+l*biss.y;
				rightEnNo=rightStNo=dest->AddPoint(x,y);
				} else {*/
		rightStNo = dest->AddPoint (pos - width*pnor);
		rightEnNo = dest->AddPoint (pos - width*nnor);
		int midNo = dest->AddPoint (pos);
		int nEdge=dest->AddEdge (rightStNo, midNo);
		if ( dest->hasBackData() ) {
			dest->ebData[nEdge].pathID=pathID;
			dest->ebData[nEdge].pieceID=pieceID;
			dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
		}
		nEdge=dest->AddEdge (midNo, rightEnNo);
		if ( dest->hasBackData() ) {
			dest->ebData[nEdge].pathID=pathID;
			dest->ebData[nEdge].pieceID=pieceID;
			dest->ebData[nEdge].tSt=dest->ebData[nEdge].tEn=tID;
		}
		//              }
	}
}


// a very ugly way to produce round joins: doing one (or two, depend on the angle of the join) quadratic bezier curves
// but since most joins are going to be small, nobody will notice -- but somebody noticed and now the ugly stuff is gone! so:

// a very nice way to produce round joins, caps or dots
void Path::RecRound(Shape *dest, int sNo, int eNo, // start and end index
        NR::Point const &iS, NR::Point const &iE, // start and end point
        NR::Point const &nS, NR::Point const &nE, // start and end normal vector
        NR::Point &origine, float width) // center and radius of round
{
    NR::Point diff = iS - iE;
    double dist = dot(diff, diff);
    if (width < 0.5 || dist/width < 2.0) {
        dest->AddEdge(sNo, eNo);
        return;
    }
    double ang, sia, lod;
    if (nS == -nE) {
        ang = M_PI;
        sia = 1;
    } else {
        double coa = dot(nS, nE);
        sia = cross(nS, nE);
        ang = acos(coa);
    if ( coa >= 1 ) {
        ang = 0;
    }
    if ( coa <= -1 ) {
        ang = M_PI;
    }
    }
    lod = 10 / (width > 500 ? 510 : 10 + width); // limit detail to about 2 degrees
    ang /= lod;
    
    int nbS = (int) floor(ang);
    NR::rotate omega(((sia > 0) ? -lod : lod));
    NR::Point cur = iS - origine;
    //  StrokeNormalize(cur);
    //  cur*=width;
    int lastNo = sNo;
    for (int i = 1; i < nbS; i++) {
        cur = cur * omega;
        NR::Point m = origine + cur;
        int mNo = dest->AddPoint(m);
        dest->AddEdge(lastNo, mNo);
        lastNo = mNo;
    }
    dest->AddEdge(lastNo, eNo);
}

/*
  Local Variables:
mode:c++
c-file-style:"stroustrup"
c-file-offsets:((innamespace . 0)(inline-open . 0))
indent-tabs-mode:nil
fill-column:99
End:
 */
// vim: filetype=c++:expandtab:shiftwidth=4:tabstop=8:softtabstop=4 :
