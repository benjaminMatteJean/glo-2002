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

//Auxiliaires

/*Retourne le numéro d'Inode d'un pFilename dans le répertoire spécifier par parentIno.*/
int getInodeFromParent(const char *pFilename, int parentIno) {
  if(strcmp(pFilename, "") == 0)
    return parentIno;
  char data[BLOCK_SIZE];
  
  int inoBNum = BASE_BLOCK_INODE;
  if((parentIno / NUM_INODE_PER_BLOCK) >= 1)
    inoBNum++;

  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  //Position de l'inode du parent.
  UINT16 inoPos = parentIno % NUM_INODE_PER_BLOCK;
  //Nombre d'entrées dans le répertoire parent.
  UINT16 nEntries = NumberofDirEntry(pINodes[inoPos].iNodeStat.st_size);

  ReadBlock(pINodes[inoPos].Block[0], data);
  DirEntry *pDE = (DirEntry *) data;

  size_t n;
  for(n=0; n < nEntries; n++) {
    if (strcmp(pFilename, pDE[n].Filename) == 0)
      return pDE[n].iNode;
  }
  return -1; //Si le nom de fichier n'existe pas...
}

/*Retourne le numéro d'Inode de pFilename par récursion.*/
int getInode(const char *pPath, const char *pFilename, int parentIno) {
  if (parentIno == -1)
    return -1;

  char pName[FILENAME_SIZE];
  int iChar, iSlash=0;
  for (iChar =0; iChar < FILENAME_SIZE; iChar++) {
    if (pPath[iChar] == 0)
      break;
    else if (pPath[iChar] == "/" && iChar != 0)
      break;
    else if (pPath[iChar] == "/")
      iSlash++;
    else {
      pName[iChar-iSlash] = pPath[iChar];
    }
  }
  pName[iChar - iSlash] = 0;
  if (strcmp(pFilename, pName) == 0){
    return getInodeFromParent(pName, parentIno);
  }
  else {
    getInode(pPath + strlen(pName) + 1, pFilename, getInodeFromParent(pName, parentIno));
  } 
}

/*Retourne le numéro d'inode spécifié par le chemin pPath.*/
int getInodeFromPath(const char *pPath){
  if(strcmp(pPath, "/") == 0)
    return ROOT_INODE;
  char pName[FILENAME_SIZE];
  if(GetFilenameFromPath(pPath, pName) == 0)
    pName[0] = 0;
  return getInode(pPath, pName, ROOT_INODE);
}



int bd_countfreeblocks(void) {
  int freeblocks =0;
  char data[BLOCK_SIZE];
  ReadBlock(FREE_BLOCK_BITMAP, data);
  for (int i=0; i<N_BLOCK_ON_DISK; i++){
    if(data[i] != 0){
      freeblocks++;
    }
  }
  return freeblocks;
}

int bd_stat(const char *pFilename, gstat *pStat) {
	return -1;
}

int bd_create(const char *pFilename) {
  char strDir[BLOCK_SIZE];
  char strFile[FILENAME_SIZE];
  ino dirInode, fileInode = 0;

  GetDirFromPath(pFilename, strDir);
  GetFilenameFromPath(pFilename, strFile);

//TODO : Vérifier que le repertoire existe et que le fichier existe avec les Auxiliaires codés.
//Si ça passe, prendre un bloque libre + mettre le bitmap a jour (Une future auxiliaire à codé)
//Créer un iNodeEntry avec le nouveau fichier et le bloque, l'écrire sur le disque.
//Mettre à jour le iNodeEntry du directrory, en y ajoutant le DirEntry du fichier sur celui-ci. (Un autre auxiliaire à codé...)

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

