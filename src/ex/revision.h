
/* 
 * This file is auto-generated by the cmake of 
 * the extension plugin.
 */

#ifndef __REVISION_H__
#define __REVISION_H__

#define AVOGADRO_VERSION_MAJOR 1
#define AVOGADRO_VERSION_MINOR 1
#define AVOGADRO_VERSION_PATCH 0

#define AVO_VERSION_ABOVE_1_0_1 if( AVOGADRO_VERSION_MAJOR == 0 ) 0 ; \
                                else if( AVOGADRO_VERSION_MAJOR>=1 && AVOGADRO_VERSION_MINOR>=0 && AVOGADRO_VERSION_PATCH<=1 ) 0 ; \
                                else if( AVOGADRO_VERSION_MAJOR>=1 && AVOGADRO_VERSION_MINOR>=0 && AVOGADRO_VERSION_PATCH>1 ) 1 ; \
                                else if( AVOGADRO_VERSION_MAJOR>=2 ) 1 ;
