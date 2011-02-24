
#ifndef __wiwo__h__
#define __wiwo__h__

#include<iostream>

using namespace std ;

/**
 * wiwo for : What's In, What's Out.
 * The class lets to use the array either like a FIFO either like a LIFO either both.
 * Tableau circulaire !! => � traduire et l'indiquer dans le nom de la classe et du
 * fichier !!
 */
template<class T> class WIWO
{

private :

  T *file ;
  int tete, nbCaseUse ;
  int tailleWIWO ;

public :

  /// Constructeur par defaut, et destructeur.

  WIWO( int tailleStock ) : tete(0), nbCaseUse(0), tailleWIWO(tailleStock)
  {
    file = new T[tailleWIWO] ;
  } ;

  ~WIWO(){ delete[] file ; } ;


  /// Assesseurs.

  int getTailleCycle(){ return tailleWIWO ; } ;
  T getTete(){ return file[tete] ; } ;
  T getQueue(){ return file[(tete+nbCaseUse-1)%tailleWIWO] ; } ;
  int getNbCaseUse(){ return nbCaseUse ; }
  void RAZ(){ nbCaseUse = 0 ; } ;


  /// M�thodes publiques.

  /**
   * Test si la file est vide.
   */
  bool isEmpty(){ return (nbCaseUse==0) ; } ;

  /**
   * Y'a 'core d'la place ?
   */
  bool isFull(){ return (nbCaseUse>=tailleWIWO) ; } ;

  /**
   * Enfiler une donn�e consiste simplement � recopier la donn�e.
   * <br/>Une copie compl�te de l'objet se fera si l'op�rateur '=' est surcharg�.
   * @param obj Objet de type T � enfiler.
   */
  void pushAtEnd( T obj )
  {
    if( nbCaseUse < tailleWIWO )
      nbCaseUse++ ;

    file[(tete+nbCaseUse) % tailleWIWO] = obj ;
  } ;


  /**
   *
   * @param obj Objet de type T � empiler.
   */
  void pushAtBegin( T obj )
  {
    if( (--tete) < 0 )
      tete = tailleWIWO-1 ;

    if( nbCaseUse < tailleWIWO )
      nbCaseUse++ ;

    file[tete] = obj ;
  } ;

  /**
   * Retirer le 1er objet.
   * <br/>Ni �crasement, ni d�sallocation, ni initialisation n'est r�alis�e
   * <br/>pour "supprimer d�finitivement" l'ancien objet de t�te.
   */
  void/*T*/ pull() // == defiler
  {
    if( nbCaseUse > 0 )
    {
      tete = (tete+1) % tailleWIWO ;
      nbCaseUse -- ;
    }

    //return file[((tete-1)<0?tete-1:tailleWIWO-1)] ;
  } ;

  T& operator [](unsigned int n)
  {
    return file[(tete+n)%tailleWIWO] ;
  };

  template<class TT> friend ostream& operator << ( ostream &o, const WIWO<TT> &f )
  {
    for( int i=0 ; i<f.nbCaseUse ; i++ )
      o << f.file[(f.tete+i)%f.tailleWIWO] << " " ;
    o<<endl ;

    return o ;
  } ;
};

#endif

