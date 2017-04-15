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

/*Retourne le numéro d'Inode d'un pFilename dans le répertoire spécifié par parentIno.*/
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
  if (parentIno == -1) {
    return -1;
  }
  char pName[FILENAME_SIZE];
  int iChar, iSlash=0;
  for (iChar =0; iChar < FILENAME_SIZE; iChar++) {
    if (pPath[iChar] == 0){
      break;
    }
    else if (pPath[iChar] == '/' && iChar != 0){
      break;
    }
    else if (pPath[iChar] == '/'){
      iSlash++;
    }
    else {
      pName[(iChar-iSlash)] = pPath[iChar];
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

/*Retourne le numéro d'un bloc libre, -1 sinon. */
int takeFreeBlock() {
  char data[BLOCK_SIZE];
  ReadBlock(FREE_BLOCK_BITMAP, data);
  int numBlock=ROOT_INODE;
  
  while(data[numBlock] == 0 && numBlock > N_BLOCK_ON_DISK) {
    numBlock++;
  }
  if(numBlock >= N_BLOCK_ON_DISK) {
    return -1;
  }

  data[numBlock] = 0;
  printf("GLOFS: Saisie bloc %d\n",numBlock);
  WriteBlock(FREE_BLOCK_BITMAP, data);
  return numBlock;
}

/*Rend le bloc BlockNum libre sur le bitmap*/
int ReleaseFreeBlock(UINT16 BlockNum) {
  char BlockFreeBitmap[BLOCK_SIZE];
  ReadBlock(FREE_BLOCK_BITMAP, BlockFreeBitmap);
  BlockFreeBitmap[BlockNum] = 1;
  printf("GLOFS: Relache bloc %d\n",BlockNum);
  WriteBlock(FREE_BLOCK_BITMAP, BlockFreeBitmap);
  return 1;
}

/*Retourne le numéro d'un inode libre.*/
int takeFreeInode(){
  char data[BLOCK_SIZE];
  ReadBlock(FREE_INODE_BITMAP, data);
  int numInode=ROOT_INODE;
  
  while(data[numInode] == 0 && numInode < N_INODE_ON_DISK) {
    numInode++;
  }
  if(numInode >= N_BLOCK_ON_DISK) {
    return -1;
  }

  data[numInode] = 0;
  printf("GLOFS: Saisie de l'i-node %d\n",numInode);
  WriteBlock(FREE_BLOCK_BITMAP, data);
  return numInode;
}

/*Retourne le iNodeEntry correspondant au numéro d'inode valide donné. -1 sinon.*/
int getInodeEntry(int ino, iNodeEntry *pIE) {
  if(ino > N_BLOCK_ON_DISK || ino < 0) {
    return -1;
  }
  char data[BLOCK_SIZE];
  int inoNum = ino;
  int inoBNum = BASE_BLOCK_INODE;
  if((inoNum / NUM_INODE_PER_BLOCK) >= 1)
    inoBNum++;

  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  //Position de l'inode du parent.
  UINT16 inoPos = inoNum % NUM_INODE_PER_BLOCK;
  *pIE = pINodes[inoPos];
  return 0;
}

/*Écris le inode passé en paramètre sur le disque */
void writeInode(iNodeEntry *pIE) {
  char data[BLOCK_SIZE];
  int inoBNum = BASE_BLOCK_INODE;
  if((pIE->iNodeStat.st_ino / NUM_INODE_PER_BLOCK) >= 1)
    inoBNum++;

  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  UINT16 inoPos = pIE->iNodeStat.st_ino % NUM_INODE_PER_BLOCK;
  pINodes[inoPos] = *pIE;
  WriteBlock(inoBNum, data);
}

/*Ajoute le directory d'un filename dans un directory existant.*/
void addFileDirInDir(iNodeEntry * destDir, ino fileIno, char * filename) {
  DirEntry *pDirEntry;
  char data[BLOCK_SIZE];

  destDir->iNodeStat.st_size += sizeof(DirEntry);
  writeInode(destDir);

  int nEntries = NumberofDirEntry(destDir->iNodeStat.st_size);
  UINT16 blNum = destDir->Block[0];
  ReadBlock(blNum, data);
  pDirEntry = (DirEntry *) data;

  pDirEntry += (nEntries -1); //Sa place dans le bloc.
  pDirEntry->iNode = fileIno;
  strcpy(pDirEntry->Filename, filename);
  WriteBlock(blNum, data);
}

/*Fin */


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
  ino fileInode = getInodeFromPath(pFilename);
  if (fileInode == -1) {
    printf("fileInode == -1");
    return -1; //N'existe pas.
  }

  iNodeEntry pInodeFile;
  if(getInodeEntry(fileInode, &pInodeFile) != 0) {
    return -1;
  }
  *pStat = pInodeFile.iNodeStat;
  return 0; //Succès.
}

int bd_create(const char *pFilename) {
  char strDir[BLOCK_SIZE];
  char strFile[FILENAME_SIZE];
  ino dirInode, fileInode = 0;

  GetDirFromPath(pFilename, strDir);
  GetFilenameFromPath(pFilename, strFile);

  dirInode = getInodeFromPath(strDir);
  if(dirInode  == -1) {
    return -1; //Le directory n'existe pas.
  }

  fileInode = getInodeFromPath(strFile);
  if(fileInode != -1) {
    return -2; //Le fichier existe déjà.
  }

  fileInode = takeFreeInode();
  iNodeEntry pInodeFile;
  getInodeEntry(fileInode,&pInodeFile);
  pInodeFile.iNodeStat.st_ino = fileInode;
  pInodeFile.iNodeStat.st_mode = G_IFREG;
  pInodeFile.iNodeStat.st_nlink = 1;
  pInodeFile.iNodeStat.st_size =0;
  pInodeFile.iNodeStat.st_blocks = 0;
  pInodeFile.iNodeStat.st_mode |=G_IRWXU | G_IRWXG;
  writeInode(&pInodeFile);

  iNodeEntry pInodeDir;
  getInodeEntry(dirInode, &pInodeDir);
  addFileDirInDir(&pInodeDir, fileInode, strFile);
  
  return 0; //ça passe
}

int bd_read(const char *pFilename, char *buffer, int offset, int numbytes) {
  ino iNodeNumber = getInodeFromPath(pFilename);
  printf("iNodeNumber du fichier: %i", iNodeNumber);
  if (iNodeNumber == -1)
    return -1; //Le fichier n'existe pas.

  iNodeEntry fileiNode;
  getInodeEntry(iNodeNumber, &fileiNode);

  if (fileiNode.iNodeStat.st_mode & G_IFDIR)
    return -2; //Ce n'est pas un fichier, mais un répertoire.
  
  int fileSize = fileiNode.iNodeStat.st_size;
  if(offset >= fileSize)
    return 0; //Incapable de lire, car position de départ plus élevée que la taille du fichier.

  /*Ici, on ajuste la taille du fichier à lire. C'est-à-dire, si offset+numbytes est plus grand
    que la taille du fichier, on réduit numbytes pour que offset+numbytes == fileSize.*/
  if ((offset + numbytes) > fileSize)
    numbytes = fileSize - offset;

  char fileData[BLOCK_SIZE];
  ReadBlock(fileiNode.Block[0], fileData);
  for (int i = offset; i < (offset + numbytes); i++)
  {
    buffer[i] = fileData[i];
  }
  return numbytes;
}

int bd_mkdir(const char *pDirName) {
  char strSubDir[FILENAME_SIZE];
  char strFilename[FILENAME_SIZE];
  ino subDirIno, dirNameIno;
  if(GetDirFromPath(pDirName, strSubDir) == 0) {
    return -1; //pDirName ne contient aucun /.
  }
  if(GetFilenameFromPath(pDirName, strFilename) == 0) {
    return -1; //Invalide.
  }
  subDirIno = getInodeFromPath(strSubDir);
  if(subDirIno == -1) {
    return -1; //Le sub directory n'existe pas.
  }
  dirNameIno = getInodeFromPath(pDirName);
  if(dirNameIno != -1) {
    return -2; //Le nouveau directory existe déjà.
  }
  iNodeEntry pInodeSubDir;
  if(getInodeEntry(subDirIno,&pInodeSubDir)== -1){
    return -1; //Invalide iNodeEntry
  }
  if(pInodeSubDir.iNodeStat.st_mode & G_IFREG) {
    return -1; //Sub directory n'est pas un répertoire.
  }

  dirNameIno = takeFreeInode();
  if(dirNameIno == -1) {
    return -1; //Plein.
  }
  int blNum  = takeFreeBlock();
  if( blNum == -1){
    return -1; //Plein.
  }
  iNodeEntry pInodeDir;
  getInodeEntry(dirNameIno,&pInodeDir);

  //incrémente le nb de liens + écrit sur disque et ajoute un directory dans le directory parent.
  pInodeSubDir.iNodeStat.st_nlink++;
  writeInode(&subDirIno);
  addFileDirInDir(&pInodeSubDir, dirNameIno, strFilename);
  //Setup des stats et ajout des repo . et .. sur le bloque.
  pInodeDir.Block[0] = blNum;
  pInodeDir.iNodeStat.st_ino = dirNameIno;
  pInodeDir.iNodeStat.st_mode = G_IFDIR;
  pInodeDir.iNodeStat.st_nlink = 2;
  pInodeDir.iNodeStat.st_size = sizeof(DirEntry) * 2;
  pInodeDir.iNodeStat.st_blocks = 1;
  pInodeDir.iNodeStat.st_mode|=G_IRWXU|G_IRWXG;
  writeInode(&pInodeDir);
  char block[BLOCK_SIZE];
  ReadBlock(blNum, block);
  DirEntry *pDir = (DirEntry *) block;
  pDir->iNode = dirNameIno;
  strcpy(pDir->Filename, ".");
  pDir++;
  pDir->iNode = subDirIno;
  strcpy(pDir->Filename, "..");
  WriteBlock(blNum, block);
  
  return 0;
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
  ino iNodeNumber = getInodeFromPath(pDirLocation);
  if (iNodeNumber == -1)
    return -1; //Directory inexistant.

  iNodeEntry diriNode;
  getInodeEntry(iNodeNumber, &diriNode);
  if (!diriNode.iNodeStat.st_mode & G_IFDIR)
    return -1; // Le fichier spécifié n'est pas un directory.
  
  char dirEntries[BLOCK_SIZE];
  ReadBlock(diriNode.Block[0], dirEntries);
  int dirSize = diriNode.iNodeStat.st_size;

  *ppListeFichiers = (DirEntry*) malloc(dirSize);
	memcpy((*ppListeFichiers), dirEntries, dirSize);

  return NumberofDirEntry(dirSize);
}

int bd_symlink(const char *pPathExistant, const char *pPathNouveauLien) {
    return -1;
}

int bd_readlink(const char *pPathLien, char *pBuffer, int sizeBuffer) {
    return -1;
}
