/***
*    YASE (Yet Another Search Engine) 
*    Copyright (C) 2000-2003  Dibyendu Majumdar.
*
*    This program is free software; you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation; either version 2 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program; if not, write to the Free Software
*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*
*    Author : Dibyendu Majumdar
*    Email  : dibyendu@mazumdar.demon.co.uk
*    Website: www.mazumdar.demon.co.uk/yase_index.html
*/
#ifndef formulas_h
#define formulas_h

inline double
/* Calculate log of document term frequency  - number of times the term appears in the document */
ys_log_dtf(
	ys_doccnt_t dtf, /* document term frequency - number of times the term appears in the document */
	ys_doccnt_t maxdtf /* maximum document term frequency - maximum number of times any term appears in a single document */
	) 
{
	if (maxdtf == 0) {
		return 1.0 + log((double)dtf);	/* MG */
		// return 1.0 + log(dtf);	/* MG */
		// return log(1.0 + dtf);
		// return log(1.0 + (double)dtf);
		// return (double)dtf;	/* SALTON */
	}
	else {
		return 0.5 + 0.5 * (double)dtf/(double)maxdtf;
	}
}

inline double
/* Calculate inverse document frequency */
ys_idf(
       ys_doccnt_t N, /* Number of documents in collection */
       ys_doccnt_t tf, /* term frequency - number of documents a term appears in */
       ys_doccnt_t maxtf /* maximum term frequency - maximum number of documents any single term appears in */
       )
{
	// return log(1.0 + (double)N/(double)tf);	/* MG */
	return log(1.0 + (double)maxtf/(double)tf);	/* Sparck Jones */
	// return log(1.0 + N/tf);
	// return log(1.0 + (double)N/(double)tf);
	// return 1.0 + log((double)N/(double)tf);	/* Sparck Jones 1972 */
	// return log((double)N/(double)tf);	/* SALTON */
}

inline double
/* Calculate document term weight - a term's weight with respect to a document */
ys_dtw(
       ys_doccnt_t N, /* Number of documents in collection */
       ys_doccnt_t tf, /* term frequency - number of documents a term appears in */
       ys_doccnt_t maxtf, /* maximum term frequency - maximum number of documents any single term appears in */
       ys_doccnt_t dtf, /* document term frequency - number of times the term appears in the document */
       ys_doccnt_t maxdtf /* maximum document term frequency - maximum number of times any term appears in a single document */
       )
{
	return ys_idf(N, tf, maxtf) * ys_log_dtf(dtf, maxdtf);
}

inline double
/* Calculate document term weight - a term's weight with respect to a document */
ys_dtw(
       double idf, /* term's inverse document frequency */
       ys_doccnt_t dtf, /* document term frequency - number of times the term appears in the document */
       ys_doccnt_t maxdtf /* maximum document term frequency - maximum number of times any term appears in a single document */
       )
{
	// return idf * ys_log_dtf(dtf);
	return idf * ys_log_dtf(dtf, maxdtf);
}


inline double
/* Calculate query term weight - a term's weight with respect to a query */
ys_qtw(
       double idf, /* term's inverse document frequency */
       int qtf, /* query term frequency - Number of times the term appears in the query */
       int qmf /* query max frequency - Maximum number of times any term appears in the query */
       )
{
	return (0.5 + 0.5 * (double)qtf/(double)qmf) * idf;
}

inline double
/* Calculate inner product */
ys_inner_product(
		 double dtw, /* term's inverse document frequency */
		 double qtw /* query term weight - a term's weight with respect to a query */
		 )
{
	return dtw * qtw;
}

#endif

