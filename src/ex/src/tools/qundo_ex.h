 
#include <QUndoCommand>

// Avoir une donn�e de retour dans le constructeur
// + http://lynxline.com/undo-in-complex-qt-projects/
// + des pointeurs sur fonction en param�tre pour �viter d'avoir des classes 
//    en doublon (pour les m�thodes avec les m�mes prototypes).
 class QUndoCommand_ex : public QUndoCommand
 {
 public:
     QUndoCommand_ex( Molecule *molecule, void* donneeDeRetour );
     void undo();
     void redo();

     void *getDonneeDeRetour() ;

 private :
   void *donneeDeRetour ;
 };
