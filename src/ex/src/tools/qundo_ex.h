 
#include <QUndoCommand>

// Avoir une donnée de retour dans le constructeur
// + http://lynxline.com/undo-in-complex-qt-projects/
// + des pointeurs sur fonction en paramètre pour éviter d'avoir des classes 
//    en doublon (pour les méthodes avec les mêmes prototypes).
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
