#include "UFS.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "disque.h"

// Quelques fonctions qui pourraient vous être utiles
int NumberofDirEntry(int Size) {
	return Size/sizeof(DirEntry);
}

int min(int a, int b) {
	return a<b ? a : b;
}

int max(int a, int b) {
	return a>b ? a : b;
}

/* Cette fonction va extraire le repertoire d'une chemin d'acces complet, et le copier
   dans pDir.  Par exemple, si le chemin fourni pPath="/doc/tmp/a.txt", cette fonction va
   copier dans pDir le string "/doc/tmp" . Si le chemin fourni est pPath="/a.txt", la fonction
   va retourner pDir="/". Si le string fourni est pPath="/", cette fonction va retourner pDir="/".
   Cette fonction est calquée sur dirname, que je ne conseille pas d'utiliser car elle fait appel
   à des variables statiques/modifie le string entrant. Voir plus bas pour un exemple d'utilisation. */
int GetDirFromPath(const char *pPath, char *pDir) {
	strcpy(pDir,pPath);
	int len = strlen(pDir); // length, EXCLUDING null
	int index;

	// On va a reculons, de la fin au debut
	while (pDir[len]!='/') {
		len--;
		if (len <0) {
			// Il n'y avait pas de slash dans le pathname
			return 0;
		}
	}
	if (len==0) {
		// Le fichier se trouve dans le root!
		pDir[0] = '/';
		pDir[1] = 0;
	}
	else {
		// On remplace le slash par une fin de chaine de caractere
		pDir[len] = '\0';
	}
	return 1;
}

/* Cette fonction va extraire le nom de fichier d'une chemin d'acces complet.
   Par exemple, si le chemin fourni pPath="/doc/tmp/a.txt", cette fonction va
   copier dans pFilename le string "a.txt" . La fonction retourne 1 si elle
   a trouvée le nom de fichier avec succes, et 0 autrement. Voir plus bas pour
   un exemple d'utilisation. */
int GetFilenameFromPath(const char *pPath, char *pFilename) {
	// Pour extraire le nom de fichier d'un path complet
	char *pStrippedFilename = strrchr(pPath,'/');
	if (pStrippedFilename!=NULL) {
		++pStrippedFilename; // On avance pour passer le slash
		if ((*pStrippedFilename) != '\0') {
			// On copie le nom de fichier trouve
			strcpy(pFilename, pStrippedFilename);
			return 1;
		}
	}
	return 0;
}


/* Un exemple d'utilisation des deux fonctions ci-dessus :
int bd_create(const char *pFilename) {
	char StringDir[256];
	char StringFilename[256];
	if (GetDirFromPath(pFilename, StringDir)==0) return 0;
	GetFilenameFromPath(pFilename, StringFilename);
	                  ...
*/


/* Cette fonction sert à afficher à l'écran le contenu d'une structure d'i-node */
void printiNode(iNodeEntry iNode) {
	printf("\t\t========= inode %d ===========\n",iNode.iNodeStat.st_ino);
	printf("\t\t  blocks:%d\n",iNode.iNodeStat.st_blocks);
	printf("\t\t  size:%d\n",iNode.iNodeStat.st_size);
	printf("\t\t  mode:0x%x\n",iNode.iNodeStat.st_mode);
	int index = 0;
	for (index =0; index < N_BLOCK_PER_INODE; index++) {
		printf("\t\t      Block[%d]=%d\n",index,iNode.Block[index]);
	}
}


/* ----------------------------------------------------------------------------------------
				  à vous de jouer, maintenant!
   ---------------------------------------------------------------------------------------- */
					 

int bd_countfreeblocks(void) {
  int freeblocks =0;
  for (int i=0; i<N_BLOCK_ON_DISK; i++){
    char pBuffer[BLOCK_SIZE];
    ReadBlock(i, pBuffer);
    if(pBuffer[0] == '\0'){
      freeblocks++;
    }
  }
  return freeblocks;
}

int bd_stat(const char *pFilename, gstat *pStat) {
  gstat temp;
  iNodeEntry node;
  getiNodeFromFilename(pFilename, node);
  return -1;
}

int bd_create(const char *pFilename) {
  char data[BLOCK_SIZE];
  ReadBlock(FREE_INODE_BITMAP, data);
  char inodes1[BLOCK_SIZE];
  ReadBlock(4, inodes1);
  char inodes2[BLOCK_SIZE];
  ReadBlock(5, inodes2);
  for(int i=0; i < N_INODE_ON_DISK; i++) {
    if( i <= 15 && data[i] == 0){
      for(int j = i*NUM_INODE_PER_BLOCK; j < ((i+1)*NUM_INODE_PER_BLOCK) -1; j++) {			
	//TODO: Faire un iNodeEntry avec les 16 bits de l`intervalle calcule 
      }
    }
  }	
  return -1;
}

int bd_read(const char *pFilename, char *buffer, int offset, int numbytes) {
	return -1;
}

int bd_mkdir(const char *pDirName) {
	return -1;
}

int bd_write(const char *pFilename, const char *buffer, int offset, int numbytes) { 
	return -1;
}

int bd_hardlink(const char *pPathExistant, const char *pPathNouveauLien) {
	return -1;
}

int bd_unlink(const char *pFilename) {
	return -1;
}

int bd_truncate(const char *pFilename, int NewSize) {
	return -1;
}

int bd_rmdir(const char *pFilename) {
	return -1;
}

int bd_rename(const char *pFilename, const char *pDestFilename) {
	return -1;
}

int bd_readdir(const char *pDirLocation, DirEntry **ppListeFichiers) {
	return -1;
}

int bd_symlink(const char *pPathExistant, const char *pPathNouveauLien) {
    return -1;
}

int bd_readlink(const char *pPathLien, char *pBuffer, int sizeBuffer) {
    return -1;
}

/* ------------------------------------------------------------------------------------------
					Fonctions utilitaires
   ------------------------------------------------------------------------------------------ */

/**
 * Permet d'écrire dans iNode le inode associé 
 */
int getiNodeFromFilename(const char *pPath, iNodeEntry *iNode) {
  char filename;
  GetFilenameFromPath(pPath, filename);
  for (int i = 4; i < 6; i++) {
    for (int j = 0; j < NUM_INODE_PER_BLOCK; j++) {
      char data[BLOCK_SIZE];
      ReadBlock(i, data);
      //TODO: Caster les données du bloc en iNodes. Comparer les noms de fichiers et directories.
    }
  }
}

