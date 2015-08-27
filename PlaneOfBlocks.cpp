// Author: Manao
// Copyright(c)2006 A.G.Balakhnin aka Fizick - global motion, overlap,  mode, refineMVs
// See legal notice in Copying.txt for more information

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA, or visit
// http://www.gnu.org/copyleft/gpl.html .

#include "VSHelper.h"

#include "PlaneOfBlocks.h"
#include "Padding.h"

#define max(a,b) ((a) > (b) ? (a) : (b))
#define min(a,b) ((a) > (b) ? (b) : (a))

PlaneOfBlocks::PlaneOfBlocks(int _nBlkX, int _nBlkY, int _nBlkSizeX, int _nBlkSizeY, int _nPel, int _nLevel, int _nMotionFlags, int _nOverlapX, int _nOverlapY, int _xRatioUV, int _yRatioUV)
{

    /* constant fields */

    nPel = _nPel;
    nLogPel = ilog2(nPel);
    // nLogPel=0 for nPel=1, 1 for nPel=2, 2 for nPel=4, i.e. (x*nPel) = (x<<nLogPel)
    nLogScale = _nLevel;
    nScale = iexp2(nLogScale);

    nBlkSizeX = _nBlkSizeX;
    nBlkSizeY = _nBlkSizeY;
    nOverlapX = _nOverlapX;
    nOverlapY = _nOverlapY;

    nBlkX = _nBlkX;
    nBlkY = _nBlkY;
    nBlkCount = nBlkX * nBlkY;

    nMotionFlags = _nMotionFlags;
    xRatioUV = _xRatioUV;
    yRatioUV = _yRatioUV;
    nLogxRatioUV = ilog2(xRatioUV);
    nLogyRatioUV = ilog2(yRatioUV);

    smallestPlane = !!(nMotionFlags & MOTION_SMALLEST_PLANE);
    chroma = !!(nMotionFlags & MOTION_USE_CHROMA_MOTION);

    globalMVPredictor.x = zeroMV.x;
    globalMVPredictor.y = zeroMV.y;
    globalMVPredictor.sad = zeroMV.sad;

    /* arrays memory allocation */

    vectors = new VECTOR[nBlkCount];
    memset(vectors, 0, nBlkCount*sizeof(VECTOR));

    /* function's pointers initialization */

    SADFunction sads[33][33];
    LUMAFunction lumas[33][33];
    COPYFunction blits[33][33];
    SADFunction satds[33][33];

    // valid block sizes for luma: 4x4, 8x4, 8x8, 16x2, 16x8, 16x16, 32x16, 32x32.
 
	sads[2][2] = Sad_C<2, 2, float>;
	blits[2][2] = Copy_C<2, 2, float>;

	sads[2][4] = Sad_C<2, 4, float>;
	blits[2][4] = Copy_C<2, 4, float>;

	sads[4][2] = Sad_C<4, 2, float>;
	blits[4][2] = Copy_C<4, 2, float>;

	sads[4][4] = Sad_C<4, 4, float>;
	lumas[4][4] = Luma_C<4, 4, float>;
	blits[4][4] = Copy_C<4, 4, float>;
	satds[4][4] = Satd_C<4, 4, float>;

	sads[4][8] = Sad_C<4, 8, float>;
	blits[4][8] = Copy_C<4, 8, float>;

	sads[8][1] = Sad_C<8, 1, float>;
	blits[8][1] = Copy_C<8, 1, float>;

	sads[8][2] = Sad_C<8, 2, float>;
	blits[8][2] = Copy_C<8, 2, float>;

	sads[8][4] = Sad_C<8, 4, float>;
	lumas[8][4] = Luma_C<8, 4, float>;
	blits[8][4] = Copy_C<8, 4, float>;
	satds[8][4] = Satd_C<8, 4, float>;

	sads[8][8] = Sad_C<8, 8, float>;
	lumas[8][8] = Luma_C<8, 8, float>;
	blits[8][8] = Copy_C<8, 8, float>;
	satds[8][8] = Satd_C<8, 8, float>;

	sads[8][16] = Sad_C<8, 16, float>;
	blits[8][16] = Copy_C<8, 16, float>;

	sads[16][1] = Sad_C<16, 1, float>;
	blits[16][1] = Copy_C<16, 1, float>;

	sads[16][2] = Sad_C<16, 2, float>;
	lumas[16][2] = Luma_C<16, 2, float>;
	blits[16][2] = Copy_C<16, 2, float>;

	sads[16][4] = Sad_C<16, 4, float>;
	blits[16][4] = Copy_C<16, 4, float>;

	sads[16][8] = Sad_C<16, 8, float>;
	lumas[16][8] = Luma_C<16, 8, float>;
	blits[16][8] = Copy_C<16, 8, float>;
	satds[16][8] = Satd_C<16, 8, float>;

	sads[16][16] = Sad_C<16, 16, float>;
	lumas[16][16] = Luma_C<16, 16, float>;
	blits[16][16] = Copy_C<16, 16, float>;
	satds[16][16] = Satd_C<16, 16, float>;

	sads[16][32] = Sad_C<16, 32, float>;
	blits[16][32] = Copy_C<16, 32, float>;

	sads[32][8] = Sad_C<32, 8, float>;
	blits[32][8] = Copy_C<32, 8, float>;

	sads[32][16] = Sad_C<32, 16, float>;
	lumas[32][16] = Luma_C<32, 16, float>;
	blits[32][16] = Copy_C<32, 16, float>;

	sads[32][32] = Sad_C<32, 32, float>;
	lumas[32][32] = Luma_C<32, 32, float>;
	blits[32][32] = Copy_C<32, 32, float>;
    


    SAD = sads[nBlkSizeX][nBlkSizeY];
    LUMA = lumas[nBlkSizeX][nBlkSizeY];
    BLITLUMA = blits[nBlkSizeX][nBlkSizeY];

    SADCHROMA = sads[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];
    BLITCHROMA = blits[nBlkSizeX / xRatioUV][nBlkSizeY / yRatioUV];

    SATD = satds[nBlkSizeX][nBlkSizeY];

    if ( !chroma )
        SADCHROMA = NULL;


    dctpitch = max(nBlkSizeX, 16) * 4;
#ifdef ALIGN_SOURCEBLOCK
    dctSrc = vs_aligned_malloc<uint8_t>(nBlkSizeY * dctpitch, ALIGN_PLANES);
    dctRef = vs_aligned_malloc<uint8_t>(nBlkSizeY * dctpitch, ALIGN_PLANES);

    nSrcPitch_temp[0] = nBlkSizeX * 4;
    nSrcPitch_temp[1] = nBlkSizeX / xRatioUV * 4;
    nSrcPitch_temp[2] = nSrcPitch_temp[1];

    pSrc_temp[0] = vs_aligned_malloc<uint8_t>(nBlkSizeY * nSrcPitch_temp[0], ALIGN_PLANES);
    pSrc_temp[1] = vs_aligned_malloc<uint8_t>(nBlkSizeY / yRatioUV * nSrcPitch_temp[1], ALIGN_PLANES);
    pSrc_temp[2] = vs_aligned_malloc<uint8_t>(nBlkSizeY / yRatioUV * nSrcPitch_temp[2], ALIGN_PLANES);
#else
    dctSrc = new uint8_t[nBlkSizeY*dctpitch];
    dctRef = new uint8_t[nBlkSizeY*dctpitch];
#endif

    freqSize = 8192*nPel*2;// half must be more than max vector length, which is (framewidth + Padding) * nPel
    freqArray = new int[freqSize];

    verybigSAD = (float)nBlkSizeX*nBlkSizeY;

}

PlaneOfBlocks::~PlaneOfBlocks()
{
    delete[] vectors;
    delete[] freqArray;

#ifdef ALIGN_SOURCEBLOCK
    vs_aligned_free(dctSrc);
    vs_aligned_free(dctRef);

    vs_aligned_free(pSrc_temp[0]);
    vs_aligned_free(pSrc_temp[1]);
    vs_aligned_free(pSrc_temp[2]);
#else
    delete[] dctSrc;
    delete[] dctRef;
#endif
}

void PlaneOfBlocks::SearchMVs(MVFrame *_pSrcFrame, MVFrame *_pRefFrame,
        SearchType st, int stp, float lambda, float lsad, int pnew,
        int plevel, int *out, VECTOR * globalMVec,
        short *outfilebuf, int fieldShift, DCTClass *_DCT, float * pmeanLumaChange,
        int divideExtra, int _pzero, int _pglobal, float _badSAD, int _badrange, bool meander, int *vecPrev, bool _tryMany)
{
    DCT = _DCT;
#ifdef ALLOW_DCT
    if (DCT==0)
        dctmode = 0;
    else
        dctmode = DCT->dctmode;
    dctweight16 = min(16,abs(*pmeanLumaChange)/(nBlkSizeX*nBlkSizeY)); //equal dct and spatial weights for meanLumaChange=8 (empirical)
#endif
    badSAD = _badSAD;
    badrange = _badrange;
    zeroMVfieldShifted.x = 0;
    zeroMVfieldShifted.y = fieldShift;
    zeroMVfieldShifted.sad = FakeInt (0.f);
    globalMVPredictor.x = nPel*globalMVec->x;// v1.8.2
    globalMVPredictor.y = nPel*globalMVec->y + fieldShift;
    globalMVPredictor.sad = globalMVec->sad;

    // write the plane's header
    WriteHeaderToArray(out);

    int *pBlkData = out + 1;
    temporal = (vecPrev) ? true : false;
    if (vecPrev) vecPrev += 1; // same as BlkData

    pSrcFrame = _pSrcFrame;
    pRefFrame = _pRefFrame;


    y[0] = pSrcFrame->GetPlane(YPLANE)->GetVPadding();

    if (pSrcFrame->GetMode() & UPLANE)
    {
        y[1] = pSrcFrame->GetPlane(UPLANE)->GetVPadding();
    }
    if (pSrcFrame->GetMode() & VPLANE)
    {
        y[2] = pSrcFrame->GetPlane(VPLANE)->GetVPadding();
    }

    nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
    if (chroma)
    {
        nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
        nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
    }

    nRefPitch[0] = pRefFrame->GetPlane(YPLANE)->GetPitch();
    if (chroma)
    {
        nRefPitch[1] = pRefFrame->GetPlane(UPLANE)->GetPitch();
        nRefPitch[2] = pRefFrame->GetPlane(VPLANE)->GetPitch();
    }

    searchType = st;//( nLogScale == 0 ) ? st : EXHAUSTIVE;
    nSearchParam = stp;//*nPel; // v1.8.2 - redesigned in v1.8.5

	float nLambdaLevel = lambda / (nPel * nPel);
    if (plevel==1)
        nLambdaLevel = nLambdaLevel*nScale;// scale lambda - Fizick
    else if (plevel==2)
        nLambdaLevel = nLambdaLevel*nScale*nScale;

    penaltyZero = _pzero;
    pglobal = _pglobal;
    planeSAD = 0.0;
    badcount = 0;
    tryMany = _tryMany;
    // Functions using float must not be used here

    for ( blky = 0; blky < nBlkY; blky++ )
    {
        blkScanDir = (blky%2 == 0 || meander == false) ? 1 : -1;
        // meander (alternate) scan blocks (even row left to right, odd row right to left)
        int blkxStart = (blky%2 == 0 || meander == false) ? 0 : nBlkX-1;
        if (blkScanDir==1) // start with leftmost block
        {
            x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
            if (chroma)
            {
                x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
                x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
            }
        }
        else // start with rightmost block, but it is already set at prev row
        {
            x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding() + (nBlkSizeX-nOverlapX)*(nBlkX-1);
            if (chroma)
            {
                x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding()+ ((nBlkSizeX-nOverlapX) / xRatioUV)*(nBlkX-1);
                x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding()+ ((nBlkSizeX-nOverlapX) / xRatioUV)*(nBlkX-1);
            }
        }
        for ( int iblkx = 0; iblkx < nBlkX; iblkx++ )
        {
            blkx = blkxStart + iblkx*blkScanDir;
            blkIdx = blky*nBlkX + blkx;
            iter=0;

            pSrc[0] = pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(x[0], y[0]);
            if (chroma)
            {
                pSrc[1] = pSrcFrame->GetPlane(UPLANE)->GetAbsolutePelPointer(x[1], y[1]);
                pSrc[2] = pSrcFrame->GetPlane(VPLANE)->GetAbsolutePelPointer(x[2], y[2]);
            }
#ifdef ALIGN_SOURCEBLOCK
            nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
            //create aligned copy
            BLITLUMA(pSrc_temp[0], nSrcPitch_temp[0], pSrc[0], nSrcPitch[0]);
            //set the to the aligned copy
            pSrc[0] = pSrc_temp[0];
            nSrcPitch[0] = nSrcPitch_temp[0];
            if (chroma)
            {
                nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
                nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
                BLITCHROMA(pSrc_temp[1], nSrcPitch_temp[1], pSrc[1], nSrcPitch[1]);
                BLITCHROMA(pSrc_temp[2], nSrcPitch_temp[2], pSrc[2], nSrcPitch[2]);
                pSrc[1] = pSrc_temp[1];
                pSrc[2] = pSrc_temp[2];
                nSrcPitch[1] = nSrcPitch_temp[1];
                nSrcPitch[2] = nSrcPitch_temp[2];
            }
#endif

            if ( blky == 0 )
                nLambda = 0.f;
            else
                nLambda = nLambdaLevel;

            penaltyNew = pnew; // penalty for new vector
            LSAD = lsad;    // SAD limit for lambda using
            // may be they must be scaled by nPel ?

            // decreased padding of coarse levels
            int nHPaddingScaled = pSrcFrame->GetPlane(YPLANE)->GetHPadding() >> nLogScale;
            int nVPaddingScaled = pSrcFrame->GetPlane(YPLANE)->GetVPadding() >> nLogScale;
            /* computes search boundaries */
            nDxMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedWidth() - x[0] - nBlkSizeX - pSrcFrame->GetPlane(YPLANE)->GetHPadding() + nHPaddingScaled);
            nDyMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedHeight()  - y[0] - nBlkSizeY - pSrcFrame->GetPlane(YPLANE)->GetVPadding() + nVPaddingScaled);
            nDxMin = -nPel * (x[0] - pSrcFrame->GetPlane(YPLANE)->GetHPadding() + nHPaddingScaled);
            nDyMin = -nPel * (y[0] - pSrcFrame->GetPlane(YPLANE)->GetVPadding() + nVPaddingScaled);

            /* search the mv */
            predictor = ClipMV(vectors[blkIdx]);
            if (temporal)
                predictors[4] = ClipMV(*reinterpret_cast<VECTOR*>(&vecPrev[blkIdx*N_PER_BLOCK])); // temporal predictor
            else
                predictors[4] = ClipMV(zeroMV);

            PseudoEPZSearch();

            if (outfilebuf != NULL) // write vector to outfile
            {
                outfilebuf[blkx*4+0] = bestMV.x;
                outfilebuf[blkx*4+1] = bestMV.y;
                outfilebuf[blkx*4+2] = (bestMV.sad & 0x0000ffff); // low word
                outfilebuf[blkx*4+3] = (bestMV.sad >> 16);     // high word, usually null
            }

            /* write the results */
            pBlkData[blkx*N_PER_BLOCK+0] = bestMV.x;
            pBlkData[blkx*N_PER_BLOCK+1] = bestMV.y;
            pBlkData[blkx*N_PER_BLOCK+2] = bestMV.sad;



            if (smallestPlane)
                sumLumaChange += LUMA(GetRefBlock(0,0), nRefPitch[0]) - LUMA(pSrc[0], nSrcPitch[0]);

            /* increment indexes & pointers */
            if ( iblkx < nBlkX-1 )
            {
                x[0] += (nBlkSizeX - nOverlapX) * blkScanDir;
                x[1] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
                x[2] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
            }
        }
        pBlkData += nBlkX*N_PER_BLOCK;
        if (outfilebuf != NULL) // write vector to outfile
            outfilebuf += nBlkX*4;// 4 short word per block

        y[0] += (nBlkSizeY - nOverlapY);
        y[1] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV );
        y[2] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV );
    }
    if (smallestPlane)
        *pmeanLumaChange = sumLumaChange/nBlkCount; // for all finer planes

}

void PlaneOfBlocks::RecalculateMVs(MVClipBalls & mvClip, MVFrame *_pSrcFrame, MVFrame *_pRefFrame,
	SearchType st, int stp, float lambda, int pnew, int *out,
	short *outfilebuf, int fieldShift, float thSAD, DCTClass *_DCT, int divideExtra, int smooth, bool meander)
{
	DCT = _DCT;
#ifdef ALLOW_DCT
	if (DCT == 0)
		dctmode = 0;
	else
		dctmode = DCT->dctmode;
	dctweight16 = 8.f;//min(16,abs(*pmeanLumaChange)/(nBlkSizeX*nBlkSizeY)); //equal dct and spatial weights for meanLumaChange=8 (empirical)
#endif
	zeroMVfieldShifted.x = 0;
	zeroMVfieldShifted.y = fieldShift;
	globalMVPredictor.x = 0;//nPel*globalMVec->x;// there is no global
	globalMVPredictor.y = fieldShift;//nPel*globalMVec->y + fieldShift;
	globalMVPredictor.sad = FakeInt (9999999.f);//globalMVec->sad;

	// write the plane's header
	WriteHeaderToArray(out);

	int *pBlkData = out + 1;

	pSrcFrame = _pSrcFrame;
	pRefFrame = _pRefFrame;

	x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
	y[0] = pSrcFrame->GetPlane(YPLANE)->GetVPadding();
	if (chroma)
	{
		x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
		x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
		y[1] = pSrcFrame->GetPlane(UPLANE)->GetVPadding();
		y[2] = pSrcFrame->GetPlane(VPLANE)->GetVPadding();
	}

	nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
	if (chroma)
	{
		nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
		nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
	}

	nRefPitch[0] = pRefFrame->GetPlane(YPLANE)->GetPitch();
	if (chroma)
	{
		nRefPitch[1] = pRefFrame->GetPlane(UPLANE)->GetPitch();
		nRefPitch[2] = pRefFrame->GetPlane(VPLANE)->GetPitch();
	}

	searchType = st;
	nSearchParam = stp;//*nPel; // v1.8.2 - redesigned in v1.8.5

	float nLambdaLevel = lambda / (nPel * nPel);

	// get old vectors plane
	const FakePlaneOfBlocks &plane = mvClip.GetPlane(0);
	int nBlkXold = plane.GetReducedWidth();
	int nBlkYold = plane.GetReducedHeight();
	int nBlkSizeXold = plane.GetBlockSizeX();
	int nBlkSizeYold = plane.GetBlockSizeY();
	int nOverlapXold = plane.GetOverlapX();
	int nOverlapYold = plane.GetOverlapY();
	int nStepXold = nBlkSizeXold - nOverlapXold;
	int nStepYold = nBlkSizeYold - nOverlapYold;
	int nPelold = plane.GetPel();
	int nLogPelold = ilog2(nPelold);

	// Functions using float must not be used here
	for (blky = 0; blky < nBlkY; blky++)
	{
		blkScanDir = (blky % 2 == 0 || meander == false) ? 1 : -1;
		// meander (alternate) scan blocks (even row left to right, odd row right to left)
		int blkxStart = (blky % 2 == 0 || meander == false) ? 0 : nBlkX - 1;
		if (blkScanDir == 1) // start with leftmost block
		{
			x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding();
			if (chroma)
			{
				x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding();
				x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding();
			}
		}
		else // start with rightmost block, but it is already set at prev row
		{
			x[0] = pSrcFrame->GetPlane(YPLANE)->GetHPadding() + (nBlkSizeX - nOverlapX)*(nBlkX - 1);
			if (chroma)
			{
				x[1] = pSrcFrame->GetPlane(UPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV)*(nBlkX - 1);
				x[2] = pSrcFrame->GetPlane(VPLANE)->GetHPadding() + ((nBlkSizeX - nOverlapX) / xRatioUV)*(nBlkX - 1);
			}
		}
		for (int iblkx = 0; iblkx < nBlkX; iblkx++)
		{
			blkx = blkxStart + iblkx*blkScanDir;
			blkIdx = blky*nBlkX + blkx;

			pSrc[0] = pSrcFrame->GetPlane(YPLANE)->GetAbsolutePelPointer(x[0], y[0]);
			if (chroma)
			{
				pSrc[1] = pSrcFrame->GetPlane(UPLANE)->GetAbsolutePelPointer(x[1], y[1]);
				pSrc[2] = pSrcFrame->GetPlane(VPLANE)->GetAbsolutePelPointer(x[2], y[2]);
			}
#ifdef ALIGN_SOURCEBLOCK
			nSrcPitch[0] = pSrcFrame->GetPlane(YPLANE)->GetPitch();
			//create aligned copy
			BLITLUMA(pSrc_temp[0], nSrcPitch_temp[0], pSrc[0], nSrcPitch[0]);
			//set the to the aligned copy
			pSrc[0] = pSrc_temp[0];
			nSrcPitch[0] = nSrcPitch_temp[0];
			if (chroma)
			{
				nSrcPitch[1] = pSrcFrame->GetPlane(UPLANE)->GetPitch();
				nSrcPitch[2] = pSrcFrame->GetPlane(VPLANE)->GetPitch();
				BLITCHROMA(pSrc_temp[1], nSrcPitch_temp[1], pSrc[1], nSrcPitch[1]);
				BLITCHROMA(pSrc_temp[2], nSrcPitch_temp[2], pSrc[2], nSrcPitch[2]);
				pSrc[1] = pSrc_temp[1];
				pSrc[2] = pSrc_temp[2];
				nSrcPitch[1] = nSrcPitch_temp[1];
				nSrcPitch[2] = nSrcPitch_temp[2];
			}
#endif

			if (blky == 0)
				nLambda = 0.f;
			else
				nLambda = nLambdaLevel;

			penaltyNew = pnew; // penalty for new vector
			// may be they must be scaled by nPel ?

			/* computes search boundaries */
			nDxMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedWidth() - x[0] - nBlkSizeX);
			nDyMax = nPel * (pSrcFrame->GetPlane(YPLANE)->GetExtendedHeight() - y[0] - nBlkSizeY);
			nDxMin = -nPel * x[0];
			nDyMin = -nPel * y[0];

			// get and interplolate old vectors
			int centerX = nBlkSizeX / 2 + (nBlkSizeX - nOverlapX)*blkx; // center of new block
			int blkxold = (centerX - nBlkSizeXold / 2) / nStepXold; // centerXold less or equal to new
			int centerY = nBlkSizeY / 2 + (nBlkSizeY - nOverlapY)*blky;
			int blkyold = (centerY - nBlkSizeYold / 2) / nStepYold;

			int deltaX = max(0, centerX - (nBlkSizeXold / 2 + nStepXold*blkxold)); // distance from old to new
			int deltaY = max(0, centerY - (nBlkSizeYold / 2 + nStepYold*blkyold));

			int blkxold1 = min(nBlkXold - 1, max(0, blkxold));
			int blkxold2 = min(nBlkXold - 1, max(0, blkxold + 1));
			int blkyold1 = min(nBlkYold - 1, max(0, blkyold));
			int blkyold2 = min(nBlkYold - 1, max(0, blkyold + 1));

			VECTOR vectorOld; // interpolated or nearest

			if (smooth == 1) // interpolate
			{

				VECTOR vectorOld1 = mvClip.GetBlock(0, blkxold1 + blkyold1*nBlkXold).GetMV(); // 4 old nearest vectors (may coinside)
				VECTOR vectorOld2 = mvClip.GetBlock(0, blkxold2 + blkyold1*nBlkXold).GetMV();
				VECTOR vectorOld3 = mvClip.GetBlock(0, blkxold1 + blkyold2*nBlkXold).GetMV();
				VECTOR vectorOld4 = mvClip.GetBlock(0, blkxold2 + blkyold2*nBlkXold).GetMV();

				// interpolate
				int vector1_x = vectorOld1.x*nStepXold + deltaX*(vectorOld2.x - vectorOld1.x); // scaled by nStepXold to skip slow division
				int vector1_y = vectorOld1.y*nStepXold + deltaX*(vectorOld2.y - vectorOld1.y);
				float vector1_sad = Back2FLT (vectorOld1.sad) *nStepXold + deltaX*(Back2FLT (vectorOld2.sad) - Back2FLT (vectorOld1.sad));

				int vector2_x = vectorOld3.x*nStepXold + deltaX*(vectorOld4.x - vectorOld3.x);
				int vector2_y = vectorOld3.y*nStepXold + deltaX*(vectorOld4.y - vectorOld3.y);
				float vector2_sad = Back2FLT (vectorOld3.sad) *nStepXold + deltaX*(Back2FLT (vectorOld4.sad) - Back2FLT (vectorOld3.sad));

				vectorOld.x = (vector1_x + deltaY*(vector2_x - vector1_x) / nStepYold) / nStepXold;
				vectorOld.y = (vector1_y + deltaY*(vector2_y - vector1_y) / nStepYold) / nStepXold;
				vectorOld.sad = FakeInt ((vector1_sad + deltaY*(vector2_sad - vector1_sad) / nStepYold) / nStepXold);

			}
			else // nearest
			{
				if (deltaX * 2<nStepXold && deltaY * 2<nStepYold)
					vectorOld = mvClip.GetBlock(0, blkxold1 + blkyold1*nBlkXold).GetMV();
				else if (deltaX * 2 >= nStepXold && deltaY * 2<nStepYold)
					vectorOld = mvClip.GetBlock(0, blkxold2 + blkyold1*nBlkXold).GetMV();
				else if (deltaX * 2<nStepXold && deltaY * 2 >= nStepYold)
					vectorOld = mvClip.GetBlock(0, blkxold1 + blkyold2*nBlkXold).GetMV();
				else //(deltaX*2>=nStepXold && deltaY*2>=nStepYold )
					vectorOld = mvClip.GetBlock(0, blkxold2 + blkyold2*nBlkXold).GetMV();
			}

			// scale vector to new nPel
			vectorOld.x = (vectorOld.x << nLogPel) >> nLogPelold;
			vectorOld.y = (vectorOld.y << nLogPel) >> nLogPelold;

			predictor = ClipMV(vectorOld); // predictor
			predictor.sad = FakeInt (Back2FLT (vectorOld.sad) * (nBlkSizeX*nBlkSizeY) / (nBlkSizeXold*nBlkSizeYold)); // normalized to new block size

			bestMV.x = predictor.x;
			bestMV.y = predictor.y;
			bestMV.sad = predictor.sad;

			// update SAD
#ifdef ALLOW_DCT
			if (dctmode != 0) // DCT method (luma only - currently use normal spatial SAD chroma)
			{
				// make dct of source block
				if (dctmode <= 4) //don't do the slow dct conversion if SATD used
					DCT->DCTBytes2D(pSrc[0], nSrcPitch[0], dctSrc, dctpitch);
			}
			if (dctmode >= 3) // most use it and it should be fast anyway //if (dctmode == 3 || dctmode == 4) // check it
				srcLuma = LUMA(pSrc[0], nSrcPitch[0]);
#endif

			float saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(predictor.x, predictor.y), nRefPitch[1])
				+ SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(predictor.x, predictor.y), nRefPitch[2]) : 0.f;
			float sad = LumaSAD(GetRefBlock(predictor.x, predictor.y));
			sad += saduv;
			bestMV.sad = FakeInt (sad);
			nMinCost = sad;


			if (Back2FLT (bestMV.sad) > thSAD)// if old interpolated vector is bad
			{
				// then, we refine, according to the search type
				if (searchType & ONETIME)
					for (int i = nSearchParam; i > 0; i /= 2)
						OneTimeSearch(i);

				if (searchType & NSTEP)
					NStepSearch(nSearchParam);

				if (searchType & LOGARITHMIC)
					for (int i = nSearchParam; i > 0; i /= 2)
						DiamondSearch(i);

				if (searchType & EXHAUSTIVE)
				{
					int mvx = bestMV.x;
					int mvy = bestMV.y;
					for (int i = 1; i <= nSearchParam; i++)// region is same as exhaustive, but ordered by radius (from near to far)
						ExpandingSearch(i, 1, mvx, mvy);
				}

				if (searchType & HEX2SEARCH)
					Hex2Search(nSearchParam);

				if (searchType & UMHSEARCH)
					UMHSearch(nSearchParam, bestMV.x, bestMV.y);

				if (searchType & HSEARCH)
				{
					int mvx = bestMV.x;
					int mvy = bestMV.y;
					for (int i = 1; i <= nSearchParam; i++)
					{
						CheckMV(mvx - i, mvy);
						CheckMV(mvx + i, mvy);
					}
				}

				if (searchType & VSEARCH)
				{
					int mvx = bestMV.x;
					int mvy = bestMV.y;
					for (int i = 1; i <= nSearchParam; i++)
					{
						CheckMV(mvx, mvy - i);
						CheckMV(mvx, mvy + i);
					}
				}
			}

			// we store the result
			vectors[blkIdx].x = bestMV.x;
			vectors[blkIdx].y = bestMV.y;
			vectors[blkIdx].sad = bestMV.sad;


			if (outfilebuf != NULL) // write vector to outfile
			{
				outfilebuf[blkx * 4 + 0] = bestMV.x;
				outfilebuf[blkx * 4 + 1] = bestMV.y;
				outfilebuf[blkx * 4 + 2] = (bestMV.sad & 0x0000ffff); // low word
				outfilebuf[blkx * 4 + 3] = (bestMV.sad >> 16);     // high word, usually null
			}

			/* write the results */
			pBlkData[blkx*N_PER_BLOCK + 0] = bestMV.x;
			pBlkData[blkx*N_PER_BLOCK + 1] = bestMV.y;
			pBlkData[blkx*N_PER_BLOCK + 2] = bestMV.sad;



			if (smallestPlane)
				sumLumaChange += LUMA(GetRefBlock(0, 0), nRefPitch[0]) - LUMA(pSrc[0], nSrcPitch[0]);

			if (iblkx < nBlkX - 1)
			{
				x[0] += (nBlkSizeX - nOverlapX) * blkScanDir;
				x[1] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
				x[2] += ((nBlkSizeX - nOverlapX) >> nLogxRatioUV) * blkScanDir;
			}
		}
		pBlkData += nBlkX*N_PER_BLOCK;
		if (outfilebuf != NULL) // write vector to outfile
			outfilebuf += nBlkX * 4;// 4 short word per block

		y[0] += (nBlkSizeY - nOverlapY);
		y[1] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
		y[2] += ((nBlkSizeY - nOverlapY) >> nLogyRatioUV);
	}
}

 void PlaneOfBlocks::InterpolatePrediction(const PlaneOfBlocks &pob)
{
    int normFactor = 3 - nLogPel + pob.nLogPel;
    int mulFactor = (normFactor < 0) ? -normFactor : 0;
    normFactor = (normFactor < 0) ? 0 : normFactor;
    int normov = (nBlkSizeX - nOverlapX)*(nBlkSizeY - nOverlapY);
    int aoddx= (nBlkSizeX*3 - nOverlapX*2);
    int aevenx = (nBlkSizeX*3 - nOverlapX*4);
    int aoddy= (nBlkSizeY*3 - nOverlapY*2);
    int aeveny = (nBlkSizeY*3 - nOverlapY*4);
    // note: overlapping is still (v2.5.7) not proceerly
    for ( int l = 0, index = 0; l < nBlkY; l++ )
    {
        for ( int k = 0; k < nBlkX; k++, index++ )
        {
            VECTOR v1, v2, v3, v4;
            int i = k;
            int j = l;
            if ( i >= 2 * pob.nBlkX ) i= 2 * pob.nBlkX-1;
            if ( j >= 2 * pob.nBlkY ) j= 2 * pob.nBlkY-1;
            int offy = -1 + 2 * ( j % 2);
            int offx = -1 + 2 * ( i % 2);

            if (( i == 0 ) || (i >= 2 * pob.nBlkX - 1))
            {
                if (( j == 0 ) || ( j >= 2 * pob.nBlkY - 1))
                {
                    v1 = v2 = v3 = v4 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
                }
                else
                {
                    v1 = v2 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
                    v3 = v4 = pob.vectors[i / 2 + (j / 2 + offy) * pob.nBlkX];
                }
            }
            else if (( j == 0 ) || ( j >= 2 * pob.nBlkY - 1))
            {
                v1 = v2 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
                v3 = v4 = pob.vectors[i / 2 + offx + (j / 2) * pob.nBlkX];
            }
            else
            {
                v1 = pob.vectors[i / 2 + (j / 2) * pob.nBlkX];
                v2 = pob.vectors[i / 2 + offx + (j / 2) * pob.nBlkX];
                v3 = pob.vectors[i / 2 + (j / 2 + offy) * pob.nBlkX];
                v4 = pob.vectors[i / 2 + offx + (j / 2 + offy) * pob.nBlkX];
            }

            double temp_sad;

            if (nOverlapX == 0 && nOverlapY == 0)
            {
                vectors[index].x = 9 * v1.x + 3 * v2.x + 3 * v3.x + v4.x;
                vectors[index].y = 9 * v1.y + 3 * v2.y + 3 * v3.y + v4.y;
                temp_sad = (double)(9 * Back2FLT (v1.sad)) + 3 * Back2FLT (v2.sad) + 3 * Back2FLT (v3.sad) + Back2FLT (v4.sad);
            }
            else if (nOverlapX <= (nBlkSizeX>>1) && nOverlapY <= (nBlkSizeY>>1)) // corrected in v1.4.11
            {
                int	ax1 = (offx > 0) ? aoddx : aevenx;
                int ax2 = (nBlkSizeX - nOverlapX)*4 - ax1;
                int ay1 = (offy > 0) ? aoddy : aeveny;
                int ay2 = (nBlkSizeY - nOverlapY)*4 - ay1;
                // 64 bit so that the multiplications by the SADs don't overflow with 16 bit input.
                int64_t a11 = ax1*ay1, a12 = ax1*ay2, a21 = ax2*ay1, a22 = ax2*ay2;
                vectors[index].x = int ((a11*v1.x + a21*v2.x + a12*v3.x + a22*v4.x) /normov);
                vectors[index].y = int ((a11*v1.y + a21*v2.y + a12*v3.y + a22*v4.y) /normov);
                temp_sad = (a11* Back2FLT (v1.sad) + a21* Back2FLT (v2.sad) + a12* Back2FLT (v3.sad) + a22* Back2FLT (v4.sad)) /normov;
            }
            else // large overlap. Weights are not quite correct but let it be
            {
                // Dead branch. The overlap is no longer allowed to be more than half the block size.
                vectors[index].x = (v1.x + v2.x + v3.x + v4.x) <<2;
                vectors[index].y = (v1.y + v2.y + v3.y + v4.y) <<2;
                temp_sad = (double)(Back2FLT (v1.sad) + Back2FLT (v2.sad) + Back2FLT (v3.sad) + Back2FLT (v4.sad)) * 4;
            }
            vectors[index].x = (vectors[index].x >> normFactor) << mulFactor;
            vectors[index].y = (vectors[index].y >> normFactor) << mulFactor;
            vectors[index].sad = FakeInt (float (temp_sad / 16));
        }
    }
}

void PlaneOfBlocks::WriteHeaderToArray(int *array)
{
    array[0] = nBlkCount * N_PER_BLOCK + 1;
}

int PlaneOfBlocks::WriteDefaultToArray(int *array, int divideMode)
{
    array[0] = nBlkCount * N_PER_BLOCK + 1;
    for (int i=0; i<nBlkCount*N_PER_BLOCK; i+=N_PER_BLOCK)
    {
        array[i+1] = 0;
        array[i+2] = 0;
        array[i+3] = FakeInt (verybigSAD);
    }

    if ( nLogScale == 0 )
    {
        array += array[0];
        if (divideMode) { // reserve space for divided subblocks extra level
            array[0] = nBlkCount * N_PER_BLOCK * 4 + 1; // 4 subblocks
            for (int i=0; i<nBlkCount*4*N_PER_BLOCK; i+=N_PER_BLOCK)
            {
                array[i+1] = 0;
                array[i+2] = 0;
                array[i+3] = FakeInt (verybigSAD);
            }
            array += array[0];
        }
    }
    return GetArraySize(divideMode);
}

int PlaneOfBlocks::GetArraySize(int divideMode)
{
    int size = 0;
    size += 1;              // mb data size storage
    size += nBlkCount * N_PER_BLOCK;  // vectors, sad, luma src, luma ref, var

    if ( nLogScale == 0)
    {
        if (divideMode)
            size += 1 + nBlkCount * N_PER_BLOCK * 4; // reserve space for divided subblocks extra level
    }

    return size;
}

void PlaneOfBlocks::FetchPredictors()
{
    // Left (or right) predictor
    if ( (blkScanDir ==1 && blkx>0) || (blkScanDir ==-1 && blkx < nBlkX - 1)) predictors[1] = ClipMV(vectors[blkIdx - blkScanDir]);
    else predictors[1] = ClipMV(zeroMVfieldShifted); // v1.11.1 - values instead of pointer

    // Up predictor
    if ( blky > 0 ) predictors[2] = ClipMV(vectors[blkIdx - nBlkX]);
    else predictors[2] = ClipMV(zeroMVfieldShifted);

    // bottom-right pridictor (from coarse level)
    if (( blky < nBlkY-1 ) && ( (blkScanDir ==1 && blkx < nBlkX - 1) || (blkScanDir ==-1 && blkx>0) ))
        predictors[3] = ClipMV(vectors[blkIdx + nBlkX + blkScanDir]);
    else
        // Up-right predictor
        if (( blky > 0 ) && ( (blkScanDir ==1 && blkx < nBlkX - 1) || (blkScanDir ==-1 && blkx>0) ))
            predictors[3] = ClipMV(vectors[blkIdx - nBlkX + blkScanDir]);
        else predictors[3] = ClipMV(zeroMVfieldShifted);

    // Median predictor
    if ( blky > 0 ) // replaced 1 by 0 - Fizick
    {
        predictors[0].x = Median(predictors[1].x, predictors[2].x, predictors[3].x);
        predictors[0].y = Median(predictors[1].y, predictors[2].y, predictors[3].y);
        //      predictors[0].sad = Median(predictors[1].sad, predictors[2].sad, predictors[3].sad);
        // but it is not true median vector (x and y may be mixed) and not its sad ?!
        // we really do not know SAD, here is more safe estimation especially for phaseshift method - v1.6.0
        predictors[0].sad = FakeInt (max(Back2FLT (predictors[1].sad), max(Back2FLT (predictors[2].sad), Back2FLT (predictors[3].sad))));
    }
    else {
        //		predictors[0].x = (predictors[1].x + predictors[2].x + predictors[3].x);
        //		predictors[0].y = (predictors[1].y + predictors[2].y + predictors[3].y);
        //      predictors[0].sad = (predictors[1].sad + predictors[2].sad + predictors[3].sad);
        // but for top line we have only left predictor[1] - v1.6.0
        predictors[0].x = predictors[1].x;
        predictors[0].y = predictors[1].y;
        predictors[0].sad = predictors[1].sad;
    }

    // if there are no other planes, predictor is the median
    if ( smallestPlane ) predictor = predictors[0];
	nLambda = nLambda*LSAD / (LSAD + (predictor.sad >> 1))*LSAD / (LSAD + (predictor.sad >> 1));
	//nLambda = nLambda*LSAD / (LSAD + (Back2FLT(predictor.sad) / 2))*LSAD / (LSAD + (Back2FLT(predictor.sad) / 2));
}


void PlaneOfBlocks::Refine()
{
    // then, we refine, according to the search type
    if ( searchType & ONETIME )
        for ( int i = nSearchParam; i > 0; i /= 2 )
            OneTimeSearch(i);

    if ( searchType & NSTEP )
        NStepSearch(nSearchParam);

    if ( searchType & LOGARITHMIC )
        for ( int i = nSearchParam; i > 0; i /= 2 )
            DiamondSearch(i);

    if ( searchType & EXHAUSTIVE )
    {
        int mvx = bestMV.x;
        int mvy = bestMV.y;
        for ( int i = 1; i <= nSearchParam; i++ )// region is same as enhausted, but ordered by radius (from near to far)
            ExpandingSearch(i, 1, mvx, mvy);
    }

    if ( searchType & HEX2SEARCH )
        Hex2Search(nSearchParam);

    if ( searchType & UMHSEARCH )
        UMHSearch(nSearchParam, bestMV.x, bestMV.y);

    if ( searchType & HSEARCH )
    {
        int mvx = bestMV.x;
        int mvy = bestMV.y;
        for ( int i = 1; i <= nSearchParam; i++ )
        {
            CheckMV(mvx - i, mvy);
            CheckMV(mvx + i, mvy);
        }
    }

    if ( searchType & VSEARCH )
    {
        int mvx = bestMV.x;
        int mvy = bestMV.y;
        for ( int i = 1; i <= nSearchParam; i++ )
        {
            CheckMV(mvx, mvy - i);
            CheckMV(mvx, mvy + i);
        }
    }
}

void PlaneOfBlocks::PseudoEPZSearch()
{

    FetchPredictors();

    float sad;
    float saduv;
#ifdef ALLOW_DCT
    if ( dctmode != 0 ) // DCT method (luma only - currently use normal spatial SAD chroma)
    {
        // make dct of source block
        if (dctmode <= 4) //don't do the slow dct conversion if SATD used
            DCT->DCTBytes2D(pSrc[0], nSrcPitch[0], dctSrc, dctpitch);
    }
    if (dctmode >= 3) // most use it and it should be fast anyway //if (dctmode == 3 || dctmode == 4) // check it
        srcLuma = LUMA(pSrc[0], nSrcPitch[0]);
#endif

    // We treat zero alone
    // Do we bias zero with not taking into account distorsion ?
    bestMV.x = zeroMVfieldShifted.x;
    bestMV.y = zeroMVfieldShifted.y;
    saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(0, 0), nRefPitch[1])
        + SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(0, 0), nRefPitch[2]) : 0.f;
    sad = LumaSAD(GetRefBlock(0, zeroMVfieldShifted.y));
    sad += saduv;
    bestMV.sad = FakeInt (sad);
    nMinCost = sad + ((penaltyZero*sad)/256); // v.1.11.0.2

    VECTOR bestMVMany[8];
    float nMinCostMany[8];

    if (tryMany)
    {
        //  refine around zero
        Refine();
        CopyVector(&bestMVMany[0], &bestMV);    // save bestMV
        nMinCostMany[0] = nMinCost;
    }

    // Global MV predictor  - added by Fizick
    globalMVPredictor = ClipMV(globalMVPredictor);
    saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(globalMVPredictor.x, globalMVPredictor.y), nRefPitch[1])
        + SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(globalMVPredictor.x, globalMVPredictor.y), nRefPitch[2]) : 0.f;
    sad = LumaSAD(GetRefBlock(globalMVPredictor.x, globalMVPredictor.y));
    sad += saduv;
    float cost = sad + ((pglobal*sad)/256);

    if ( cost  < nMinCost || tryMany)
    {
        bestMV.x = globalMVPredictor.x;
        bestMV.y = globalMVPredictor.y;
        bestMV.sad = FakeInt (sad);
        nMinCost = cost;
    }
    if (tryMany)
    {
        // refine around global
        Refine();    // reset bestMV
        CopyVector(&bestMVMany[1], &bestMV);    // save bestMV
        nMinCostMany[1] = nMinCost;
    }
    saduv = (chroma) ? SADCHROMA(pSrc[1], nSrcPitch[1], GetRefBlockU(predictor.x, predictor.y), nRefPitch[1])
        + SADCHROMA(pSrc[2], nSrcPitch[2], GetRefBlockV(predictor.x, predictor.y), nRefPitch[2]) : 0.f;
    sad = LumaSAD(GetRefBlock(predictor.x, predictor.y));
    sad += saduv;
    cost = sad;

    if ( cost  < nMinCost || tryMany )
    {
        bestMV.x = predictor.x;
        bestMV.y = predictor.y;
        bestMV.sad = FakeInt (sad);
        nMinCost = cost;
    }
    if (tryMany)
    {
        // refine around predictor
        Refine();    // reset bestMV
        CopyVector(&bestMVMany[2], &bestMV);    // save bestMV
        nMinCostMany[2] = nMinCost;
    }

    // then all the other predictors
    int npred = (temporal) ? 5 : 4;

    for ( int i = 0; i < npred; i++ )
    {
        if (tryMany)
            nMinCost = verybigSAD;
        CheckMV0(predictors[i].x, predictors[i].y);
        if (tryMany)
        {
            // refine around predictor
            Refine();    // reset bestMV
            CopyVector(&bestMVMany[i+3], &bestMV);    // save bestMV
            nMinCostMany[i+3] = nMinCost;
        }
    }


    if (tryMany)
    { // select best of multi best
        nMinCost = verybigSAD;
        for (int i=0; i<npred+3; i++)
        {
            if (nMinCostMany[i] < nMinCost)
            {
                CopyVector(&bestMV, &bestMVMany[i]);
                nMinCost = nMinCostMany[i];
            }
        }
    }
    else
    {
        // then, we refine, according to the search type
        Refine();
    }

    float foundSAD = Back2FLT (bestMV.sad);

#define BADCOUNT_LIMIT 16

    if (blkIdx>1 && foundSAD > (badSAD +  badSAD*badcount/BADCOUNT_LIMIT)) // bad vector, try wide search
    {// with some soft limit (BADCOUNT_LIMIT) of bad cured vectors (time consumed)
        badcount++;

        if (badrange > 0) // UMH
        {


            {
                // rathe good is not found, lets try around zero
                UMHSearch(badrange*nPel, 0, 0);
            }

        }
        else if (badrange<0) // ESA
        {

            for ( int i = 1; i < -badrange*nPel; i+=nPel )// at radius
            {
                ExpandingSearch(i, nPel, 0, 0);
                if (Back2FLT (bestMV.sad) < foundSAD/4) break; // stop search if rathe good is found
            }
        }

        int mvx = bestMV.x; // refine in small area
        int mvy = bestMV.y;
        for ( int i = 1; i < nPel; i++ )// small radius
        {
            ExpandingSearch(i, 1, mvx, mvy);
        }
        }


        // we store the result
        vectors[blkIdx].x = bestMV.x;
        vectors[blkIdx].y = bestMV.y;
        vectors[blkIdx].sad = bestMV.sad;

        planeSAD += Back2FLT (bestMV.sad);

    }

    void PlaneOfBlocks::DiamondSearch(int length)
    {
        // The meaning of the directions are the following :
        //		* 1 means right
        //		* 2 means left
        //		* 4 means down
        //		* 8 means up
        // So 1 + 4 means down right, and so on...

        int dx;
        int dy;

        // We begin by making no assumption on which direction to search.
        int direction = 15;

        int lastDirection;

        while ( direction > 0 )
        {
            dx = bestMV.x;
            dy = bestMV.y;
            lastDirection = direction;
            direction = 0;

            // First, we look the directions that were hinted by the previous step
            // of the algorithm. If we find one, we add it to the set of directions
            // we'll test next
            if ( lastDirection & 1 ) CheckMV2(dx + length, dy, &direction, 1);
            if ( lastDirection & 2 ) CheckMV2(dx - length, dy, &direction, 2);
            if ( lastDirection & 4 ) CheckMV2(dx, dy + length, &direction, 4);
            if ( lastDirection & 8 ) CheckMV2(dx, dy - length, &direction, 8);

            // If one of the directions improves the SAD, we make further tests
            // on the diagonals
            if ( direction ) {
                lastDirection = direction;
                dx = bestMV.x;
                dy = bestMV.y;

                if ( lastDirection & 3 )
                {
                    CheckMV2(dx, dy + length, &direction, 4);
                    CheckMV2(dx, dy - length, &direction, 8);
                }
                else {
                    CheckMV2(dx + length, dy, &direction, 1);
                    CheckMV2(dx - length, dy, &direction, 2);
                }
            }

            // If not, we do not stop here. We infer from the last direction the
            // diagonals to be checked, because we might be lucky.
            else {
                switch ( lastDirection ) {
                    case 1 :
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        break;
                    case 2 :
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        break;
                    case 4 :
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        break;
                    case 8 :
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        break;
                    case 1 + 4 :
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        break;
                    case 2 + 4 :
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        break;
                    case 1 + 8 :
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        break;
                    case 2 + 8 :
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        break;
                    default :
                        // Even the default case may happen, in the first step of the
                        // algorithm for example.
                        CheckMV2(dx + length, dy + length, &direction, 1 + 4);
                        CheckMV2(dx - length, dy + length, &direction, 2 + 4);
                        CheckMV2(dx + length, dy - length, &direction, 1 + 8);
                        CheckMV2(dx - length, dy - length, &direction, 2 + 8);
                        break;
                }
            }
        }
    }

    void PlaneOfBlocks::NStepSearch(int stp)
    {
        int dx, dy;
        int length = stp;
        while ( length > 0 )
        {
            dx = bestMV.x;
            dy = bestMV.y;

            CheckMV(dx + length, dy + length);
            CheckMV(dx + length, dy);
            CheckMV(dx + length, dy - length);
            CheckMV(dx, dy - length);
            CheckMV(dx, dy + length);
            CheckMV(dx - length, dy + length);
            CheckMV(dx - length, dy);
            CheckMV(dx - length, dy - length);

            length--;
        }
    }

    void PlaneOfBlocks::OneTimeSearch(int length)
    {
        int direction = 0;
        int dx = bestMV.x;
        int dy = bestMV.y;

        CheckMV2(dx - length, dy, &direction, 2);
        CheckMV2(dx + length, dy, &direction, 1);

        if ( direction == 1 )
        {
            while ( direction )
            {
                direction = 0;
                dx += length;
                CheckMV2(dx + length, dy, &direction, 1);
            }
        }
        else if ( direction == 2 )
        {
            while ( direction )
            {
                direction = 0;
                dx -= length;
                CheckMV2(dx - length, dy, &direction, 1);
            }
        }

        CheckMV2(dx, dy - length, &direction, 2);
        CheckMV2(dx, dy + length, &direction, 1);

        if ( direction == 1 )
        {
            while ( direction )
            {
                direction = 0;
                dy += length;
                CheckMV2(dx, dy + length, &direction, 1);
            }
        }
        else if ( direction == 2 )
        {
            while ( direction )
            {
                direction = 0;
                dy -= length;
                CheckMV2(dx, dy - length, &direction, 1);
            }
        }
    }

    void PlaneOfBlocks::ExpandingSearch(int r, int s, int mvx, int mvy) // diameter = 2*r + 1, step=s
    { // part of true enhaustive search (thin expanding square) around mvx, mvy
        int i, j;

        // sides of square without corners
        for ( i = -r+s; i < r; i+=s ) // without corners! - v2.1
        {
            CheckMV(mvx + i, mvy - r);
            CheckMV(mvx + i, mvy + r);
        }

        for ( j = -r+s; j < r; j+=s )
        {
            CheckMV(mvx - r, mvy + j);
            CheckMV(mvx + r, mvy + j);
        }

        // then corners - they are more far from cenrer
        CheckMV(mvx - r, mvy - r);
        CheckMV(mvx - r, mvy + r);
        CheckMV(mvx + r, mvy - r);
        CheckMV(mvx + r, mvy + r);
    }

    /* (x-1)%6 */
    static const int mod6m1[8] = {5,0,1,2,3,4,5,0};
    /* radius 2 hexagon. repeated entries are to avoid having to compute mod6 every time. */
    static const int hex2[8][2] = {{-1,-2}, {-2,0}, {-1,2}, {1,2}, {2,0}, {1,-2}, {-1,-2}, {-2,0}};

    void PlaneOfBlocks::Hex2Search(int i_me_range)
    { //adopted from x264
        int dir = -2;
        int bmx = bestMV.x;
        int bmy = bestMV.y;

        if (i_me_range > 1)
        {
            /* hexagon */
            //        COST_MV_X3_DIR( -2,0, -1, 2,  1, 2, costs   );
            //        COST_MV_X3_DIR(  2,0,  1,-2, -1,-2, costs+3 );
            //        COPY2_IF_LT( bcost, costs[0], dir, 0 );
            //        COPY2_IF_LT( bcost, costs[1], dir, 1 );
            //        COPY2_IF_LT( bcost, costs[2], dir, 2 );
            //        COPY2_IF_LT( bcost, costs[3], dir, 3 );
            //        COPY2_IF_LT( bcost, costs[4], dir, 4 );
            //        COPY2_IF_LT( bcost, costs[5], dir, 5 );
            CheckMVdir(bmx-2, bmy, &dir, 0);
            CheckMVdir(bmx-1, bmy+2, &dir, 1);
            CheckMVdir(bmx+1, bmy+2, &dir, 2);
            CheckMVdir(bmx+2, bmy, &dir, 3);
            CheckMVdir(bmx+1, bmy-2, &dir, 4);
            CheckMVdir(bmx-1, bmy-2, &dir, 5);


            if( dir != -2 )
            {
                bmx += hex2[dir+1][0];
                bmy += hex2[dir+1][1];
                /* half hexagon, not overlapping the previous iteration */
                for( int i = 1; i < i_me_range/2 && IsVectorOK(bmx, bmy); i++ )
                {
                    const int odir = mod6m1[dir+1];
                    //                COST_MV_X3_DIR( hex2[odir+0][0], hex2[odir+0][1],
                    //                                hex2[odir+1][0], hex2[odir+1][1],
                    //                                hex2[odir+2][0], hex2[odir+2][1],
                    //                                costs );

                    dir = -2;
                    //                COPY2_IF_LT( bcost, costs[0], dir, odir-1 );
                    //                COPY2_IF_LT( bcost, costs[1], dir, odir   );
                    //                COPY2_IF_LT( bcost, costs[2], dir, odir+1 );

                    CheckMVdir(bmx + hex2[odir+0][0], bmy + hex2[odir+0][1], &dir, odir-1);
                    CheckMVdir(bmx + hex2[odir+1][0], bmy + hex2[odir+1][1], &dir, odir);
                    CheckMVdir(bmx + hex2[odir+2][0], bmy + hex2[odir+2][1], &dir, odir+1);
                    if( dir == -2 )
                        break;
                    bmx += hex2[dir+1][0];
                    bmy += hex2[dir+1][1];
                }
            }

            bestMV.x = bmx;
            bestMV.y = bmy;
        }
        /* square refine */
        //        omx = bmx; omy = bmy;
        //        COST_MV_X4(  0,-1,  0,1, -1,0, 1,0 );
        //        COST_MV_X4( -1,-1, -1,1, 1,-1, 1,1 );
        ExpandingSearch(1, 1, bmx, bmy);

    }


    void PlaneOfBlocks::CrossSearch(int start, int x_max, int y_max, int mvx, int mvy)
    { // part of umh  search

        for ( int i = start; i < x_max; i+=2 )
        {
            CheckMV(mvx - i, mvy);
            CheckMV(mvx + i, mvy);
        }

        for ( int j = start; j < y_max; j+=2 )
        {
            CheckMV(mvx, mvy + j);
            CheckMV(mvx, mvy + j);
        }
    }


    void PlaneOfBlocks::UMHSearch(int i_me_range, int omx, int omy) // radius
    {
        // Uneven-cross Multi-Hexagon-grid Search (see x264)
        /* hexagon grid */

        //            int omx = bestMV.x;
        //            int omy = bestMV.y;
        // my mod: do not shift the center after Cross
        CrossSearch(1, i_me_range, i_me_range,  omx,  omy);


        int i = 1;
        do
        {
            static const int hex4[16][2] = {
                {-4, 2}, {-4, 1}, {-4, 0}, {-4,-1}, {-4,-2},
                { 4,-2}, { 4,-1}, { 4, 0}, { 4, 1}, { 4, 2},
                { 2, 3}, { 0, 4}, {-2, 3},
                {-2,-3}, { 0,-4}, { 2,-3},
            };

            for( int j = 0; j < 16; j++ )
            {
                int mx = omx + hex4[j][0]*i;
                int my = omy + hex4[j][1]*i;
                CheckMV( mx, my );
            }
        } while( ++i <= i_me_range/4 );

        //            if( bmy <= mv_y_max )
        //                goto me_hex2;
        Hex2Search(i_me_range);

    }
    //----------------------------------------------------------------

    void PlaneOfBlocks::EstimateGlobalMVDoubled(VECTOR *globalMVec)
    {
        // estimate global motion from current plane vectors data for using on next plane - added by Fizick
        // on input globalMVec is prev estimation
        // on output globalMVec is doubled for next scale plane using

        // use very simple but robust method
        // more advanced method (like MVDepan) can be implemented later

        // find most frequent x
        memset(&freqArray[0], 0, freqSize*sizeof(int)); // reset
        int indmin = freqSize-1;
        int indmax = 0;
        for (int i=0; i<nBlkCount; i++)
        {
            int ind = (freqSize>>1)+vectors[i].x;
            if (ind>=0 && ind<freqSize)
            {
                freqArray[ind] += 1 ;
                if (ind > indmax)
                    indmax = ind;
                if (ind < indmin)
                    indmin = ind;
            }
        }
        int count = freqArray[indmin];
        int index = indmin;
        for (int i=indmin+1; i<=indmax; i++)
        {
            if (freqArray[i] > count)
            {
                count = freqArray[i];
                index = i;
            }
        }
        int medianx = (index-(freqSize>>1)); // most frequent value

        // find most frequent y
        memset(&freqArray[0], 0, freqSize*sizeof(int)); // reset
        indmin = freqSize-1;
        indmax = 0;
        for (int i=0; i<nBlkCount; i++)
        {
            int ind = (freqSize>>1)+vectors[i].y;
            if (ind>=0 && ind<freqSize)
            {
                freqArray[ind] += 1 ;
                if (ind > indmax)
                    indmax = ind;
                if (ind < indmin)
                    indmin = ind;
            }
        }
        count = freqArray[indmin];
        index = indmin;
        for (int i=indmin+1; i<=indmax; i++)
        {
            if (freqArray[i] > count)
            {
                count = freqArray[i];
                index = i;
            }
        }
        int mediany = (index-(freqSize>>1)); // most frequent value


        // iteration to increase precision
        int meanvx = 0;
        int meanvy = 0;
        int num = 0;
        for ( int i=0; i < nBlkCount; i++ )
        {
            if (abs(vectors[i].x - medianx) < 6 && abs(vectors[i].y - mediany) < 6 )
            {
                meanvx += vectors[i].x;
                meanvy += vectors[i].y;
                num += 1;
            }
        }

        // output vectors must be doubled for next (finer) scale level
        if (num >0)
        {
            globalMVec->x = 2*meanvx / num;
            globalMVec->y = 2*meanvy / num;
        }
        else
        {
            globalMVec->x = 2*medianx;
            globalMVec->y = 2*mediany;
        }
    }