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

/* Les fonctions suivantes sont des fonctions auxiliaires qui facilitent l'accès, la lecture
 * et l'écriture des différents iNodes et blocs.
 */

/*Retourne le numéro d'un bloc libre, -1 sinon. */
int takeBlock() {
  char freeBlockBitmap[BLOCK_SIZE];
  ReadBlock(FREE_BLOCK_BITMAP, freeBlockBitmap);

  int free = -1;
  for (int i = 6; i < N_BLOCK_ON_DISK; i++) {
    if(freeBlockBitmap[i] != 0)
      free = i;
  }
  if (free == -1)
    return -1; //Aucun bloque libre!
  
  // On indique que le bloc free est maintenant saisi
  freeBlockBitmap[free] = 0;
  printf("GLOFS: Saisie bloc %d\n", free);
  WriteBlock(FREE_BLOCK_BITMAP, freeBlockBitmap);

  return free;
}

/*Retourne le numéro d'un inode libre.*/
int takeiNode(){
  char freeiNodesBitmap[BLOCK_SIZE];
  ReadBlock(FREE_INODE_BITMAP, freeiNodesBitmap);

  int free = -1;
  for (int i = 0; i < N_INODE_ON_DISK; i++) {
    if(freeiNodesBitmap[i] != 0)
      free = i;
  }
  if (free == -1)
    return -1; //Aucun i-node libre

  // On indique que l'iNode free est maintenant saisi
  freeiNodesBitmap[free] = 0;
  printf("GLOFS: Saisie i-node %d\n", free);
  WriteBlock(FREE_INODE_BITMAP, freeiNodesBitmap);

  return free;
}

/*Rend le bloc BlockNum libre sur le bitmap*/
int releaseBlock(UINT16 BlockNum) {
  char BlockFreeBitmap[BLOCK_SIZE];
  ReadBlock(FREE_BLOCK_BITMAP, BlockFreeBitmap);
  BlockFreeBitmap[BlockNum] = 1;
  printf("GLOFS: Relache bloc %d\n",BlockNum);
  WriteBlock(FREE_BLOCK_BITMAP, BlockFreeBitmap);
  return 1;
}

/*Relache l'inode passé en paramètre*/
int releaseInode(int inoNum) {
  char data[BLOCK_SIZE];
  ReadBlock(FREE_INODE_BITMAP, data);
  data[inoNum] = 1;
  printf("GLOFS: Relache i-node %d\n", inoNum);
  WriteBlock(FREE_INODE_BITMAP, data);
  return 1;
}

/*Retourne le iNodeEntry correspondant au numéro d'inode valide donné. -1 sinon.*/
int getIE(int ino, iNodeEntry *pIE) {

  if(ino > N_BLOCK_ON_DISK || ino < 0) {
    return -1; //Invalide
  }
  
  char data[BLOCK_SIZE];
  int inoNum = ino;
  int inoBNum = BASE_BLOCK_INODE;
  if((inoNum / NUM_INODE_PER_BLOCK) >= 1)
    inoBNum++; //Le numéro d'i-node dépasse 16

  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  //Position de l'inode du parent.
  int inoPos = inoNum % NUM_INODE_PER_BLOCK;
  *pIE = pINodes[inoPos];
  return 0;
}

/*Écris le inode passé en paramètre sur le disque */
void writeInode(iNodeEntry *pIE) {
  char data[BLOCK_SIZE];
  int inoBNum = BASE_BLOCK_INODE;
  if((pIE->iNodeStat.st_ino / NUM_INODE_PER_BLOCK) >= 1)
    inoBNum++; //Le numéro d'i-node dépasse 16

  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  int inoPos = pIE->iNodeStat.st_ino % NUM_INODE_PER_BLOCK;
  pINodes[inoPos] = *pIE;
  WriteBlock(inoBNum, data); //Écriture sur le disque 
}

/*Ajoute le directory d'un filename dans un directory existant.*/
void addDir(iNodeEntry * destDir, char * filename, ino fileIno) {
  DirEntry *pDirEntry;
  char data[BLOCK_SIZE];

  destDir->iNodeStat.st_size += sizeof(DirEntry); //On ajoute un entry
  writeInode(destDir);//Écriture de l'i-node sur le disque

  int nEntries = NumberofDirEntry(destDir->iNodeStat.st_size);
  UINT16 blNum = destDir->Block[0];
  ReadBlock(blNum, data);
  pDirEntry = (DirEntry *) data;

  pDirEntry += (nEntries -1); //Sa place dans le bloc.
  pDirEntry->iNode = fileIno;
  strcpy(pDirEntry->Filename, filename);
  WriteBlock(blNum, data);//Écriture du bloque DirEntry sur le disque
}

/*Enlève du dirEntry l'entry spécifiée par le numIno*/
void removeDir(iNodeEntry * iNodeDirectory, ino numIno) {
  char data[BLOCK_SIZE];
  int size  = iNodeDirectory->iNodeStat.st_size;
  iNodeDirectory->iNodeStat.st_size -= BLOCK_SIZE / sizeof(DirEntry);
  writeInode(iNodeDirectory);
  UINT16 blNum = iNodeDirectory->Block[0];
  ReadBlock(blNum, data);
  DirEntry * pDir = (DirEntry *) data;

  int i=0, count = NumberofDirEntry(size), found = 0;
  while(i < count) {
    if(pDir[i].iNode == numIno){
      found = 1;
    }
    if(found == 1) {
      pDir[i] = pDir[i+1];
    }
    i++;
  }
  WriteBlock(blNum, data);
}
/*Retourne le numéro d'Inode d'un pFilename dans le répertoire spécifié par parentIno.*/
int getChild(int parentIno, const char *pFilename) {
  char data[BLOCK_SIZE];
  int inoBNum = BASE_BLOCK_INODE;
  if((parentIno / NUM_INODE_PER_BLOCK) >= 1) //Si le parentIno est plus grand que 16
    inoBNum++;
  
  if(strcmp(pFilename, "") == 0)
    return parentIno; //Filename vide
  
  ReadBlock(inoBNum, data);
  iNodeEntry *pINodes = (iNodeEntry *) data;
  //Position de l'inode du parent.
  int inoPos = parentIno % NUM_INODE_PER_BLOCK;
  //Nombre d'entrées dans le répertoire parent.
  int nEntries = NumberofDirEntry(pINodes[inoPos].iNodeStat.st_size);


  ReadBlock(pINodes[inoPos].Block[0], data); //Bloque de donnée de l'inode parent
  DirEntry *pDE = (DirEntry *) data;
  int i;
  for(i=0; i < nEntries; i++) {
    if (strcmp(pFilename, pDE[i].Filename) == 0)
      return pDE[i].iNode; //C'est celui qu'on cherche
  }
  return -1; //Si le nom de fichier n'existe pas dans le répertoire...
}

/* Retourne le numéro d'iNode de pFilename par récursion.
 * @param pPath, le path du fichier/répertoire désiré
 * @param pFilename, le nom du fichier/répertoire
 * @param parentIno, le numéro d'iNode du parent
 */
int getInode(const char *pPath, const char *pFilename, int parentIno) {
  if (parentIno == -1) {
    return -1;
  }
  char pName[FILENAME_SIZE];
  int i, nbOfSlash=0;
  for (i=0; i < FILENAME_SIZE; i++) {
    if (pPath[i] == 0){
      break; //Path null
    }
    else if (pPath[i] == '/' && i != 0){
      break; //Lorsqu'on arrive à un sous répertoire
    }
    else if (pPath[i] == '/'){
      nbOfSlash++; //Premier slash du pPath
    }
    else {
      pName[(i-nbOfSlash)] = pPath[i]; 
    }
  }

  //Le dernier caractère a null (null-terminated string)
  pName[i-nbOfSlash] = 0;
  
  if (strcmp(pFilename, pName) == 0){ //Si pName représente le fichier lui même
    return getChild(parentIno, pName);
  }
  else { //Sinon, c'est un de ses répertoires parent
    //Récursion avec le path racourci, le même nom de fichier et l'inode du répertoire sous-répertoire trouvé par pName.
    getInode(pPath + (strlen(pName) + 1), pFilename, getChild(parentIno, pName));
  } 
}

/* Retourne le numéro d'iNode correspondant au path en paramètre.
 * @param pPath, le path du fichier/répertoire désiré
 */
int getInodeFromPath(const char *pPath){
  if(strcmp(pPath, "/") == 0)
    return ROOT_INODE;

  char filename[FILENAME_SIZE];

  if(GetFilenameFromPath(pPath, filename) == 0)
    filename[0] = 0;
  //Appel de la fonction getInode avec directory et filename de passé en paramètre.
  return getInode(pPath, filename, ROOT_INODE);
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
    return -1; //N'existe pas.
  }

  iNodeEntry pInodeFile;
  if(getIE(fileInode, &pInodeFile) != 0) {
    return -1;
  }
  *pStat = pInodeFile.iNodeStat;
  return 0; //Succes.
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

  fileInode = takeiNode(); 
  iNodeEntry pInodeFile;
  getIE(fileInode,&pInodeFile);
  //Setup des stats et écriture sur le disque
  pInodeFile.iNodeStat.st_ino = fileInode;
  pInodeFile.iNodeStat.st_mode = G_IFREG;
  pInodeFile.iNodeStat.st_nlink = 1;
  pInodeFile.iNodeStat.st_size =0;
  pInodeFile.iNodeStat.st_blocks = 0;
  pInodeFile.iNodeStat.st_mode|= G_IRWXU | G_IRWXG; 
  writeInode(&pInodeFile);

  //Ajout d'un répertoire pour le nouveau fichier.
  iNodeEntry pInodeDir;
  getIE(dirInode, &pInodeDir);
  addDir(&pInodeDir, strFile, fileInode); 
  
  return 0; //ça passe
}

int bd_read(const char *pFilename, char *buffer, int offset, int numbytes) {
  ino iNodeNumber = getInodeFromPath(pFilename);
  if (iNodeNumber == -1)
    return -1; //Le fichier n'existe pas.

  iNodeEntry fileiNode;
  getIE(iNodeNumber, &fileiNode);

  if (fileiNode.iNodeStat.st_mode & G_IFDIR)
    return -2; //Ce n'est pas un fichier, mais un répertoire.
  
  int fileSize = fileiNode.iNodeStat.st_size;
  if(offset >= fileSize)
    return 0; // Incapable de lire, car position de départ plus élevée que la taille du fichier.

  /*Ici, on ajuste la taille du fichier à lire. C'est-à-dire, si offset+numbytes est plus grand
    que la taille du fichier, on réduit numbytes pour que offset+numbytes == fileSize.*/
  if ((offset + numbytes) > fileSize)
    numbytes = fileSize - offset;

  char fileData[BLOCK_SIZE];
  ReadBlock(fileiNode.Block[0], fileData);
  for (int i = offset; i < (offset + numbytes); i++)
    buffer[i-offset] = fileData[i];

  return numbytes;
}

int bd_mkdir(const char *pDirName) {
  char strSubDir[FILENAME_SIZE];
  char strFilename[FILENAME_SIZE];
  ino subDirIno, dirNameIno;
  iNodeEntry pInodeSubDir;
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
  
  if(getIE(subDirIno,&pInodeSubDir)== -1){
    return -1; //Invalide iNodeEntry
  }
  
  if(pInodeSubDir.iNodeStat.st_mode & G_IFREG) {
    return -1; //Sub directory n'est pas un répertoire, mais un fichier régulier.
  }
  
  dirNameIno = getInodeFromPath(pDirName);
  if(dirNameIno != -1) {
    return -2; //Le nouveau directory existe déjà.
  }

  dirNameIno = takeiNode();
  if(dirNameIno == -1) {
    return -1; //Plein.
  }
  int blNum  = takeBlock();
  if( blNum == -1){
    return -1; //Plein.
  }
  iNodeEntry pInodeDir;
  getIE(dirNameIno,&pInodeDir);

  //incrémente le nb de liens + écrit sur disque et ajoute un directory dans le directory parent.
  pInodeSubDir.iNodeStat.st_nlink++;
  writeInode(&pInodeSubDir);
  addDir(&pInodeSubDir, strFilename, dirNameIno);
  //Setup des stats et ajout des repo . et .. sur le bloque.
  pInodeDir.Block[0] = blNum;
  pInodeDir.iNodeStat.st_ino = dirNameIno;
  pInodeDir.iNodeStat.st_mode = G_IFDIR;
  pInodeDir.iNodeStat.st_nlink = 2;
  pInodeDir.iNodeStat.st_size = sizeof(DirEntry) * 2; //Car il y a . et ..
  pInodeDir.iNodeStat.st_blocks = 1;
  pInodeDir.iNodeStat.st_mode|=G_IRWXU|G_IRWXG;
  writeInode(&pInodeDir);
  char datablock[BLOCK_SIZE];
  ReadBlock(blNum, datablock);
  DirEntry *pDir = (DirEntry *) datablock;
  pDir->iNode = dirNameIno;
  strcpy(pDir->Filename, ".");
  pDir++;//Prochain entry
  pDir->iNode = subDirIno;
  strcpy(pDir->Filename, "..");
  WriteBlock(blNum, datablock);
  
  return 0;
}

int bd_write(const char *pFilename, const char *buffer, int offset, int numbytes) { 
  ino iNodeNumber = getInodeFromPath(pFilename);
  if (iNodeNumber == -1)
    return -1; // Le fichier n'existe pas.

  iNodeEntry fileiNode;
  getIE(iNodeNumber, &fileiNode);

  if (fileiNode.iNodeStat.st_mode & G_IFDIR)
    return -2; // Ce n'est pas un fichier, mais un répertoire.

  int fileSize = fileiNode.iNodeStat.st_size;

  if (offset > fileSize && offset < BLOCK_SIZE*N_BLOCK_PER_INODE)
    return -3; // Impossible d'écrire, car position de départ plus élevée que la taille du fichier.

  if (offset >= BLOCK_SIZE*N_BLOCK_PER_INODE)
    return -4; // Impossible d'écrire, taille maximale allouée pour un fichier dépassée.

  // Si l'espace à écrire dépasse la limite, on réduit l'espace à écrire jusqu'à la limite permise.
  if ((offset + numbytes) >= BLOCK_SIZE*N_BLOCK_PER_INODE) {
    numbytes = BLOCK_SIZE*N_BLOCK_PER_INODE - offset;
  }
	
	// Si le fichier ne contient aucune donnée, on crée un nouveau bloc.
	if (fileiNode.iNodeStat.st_blocks == 0 && fileSize == 0) {
		int newBlock = takeBlock();
		if (newBlock == -1)
			return 0; // Aucun bloc libre disponible.
		fileiNode.Block[0] = newBlock;
		fileiNode.iNodeStat.st_blocks = 1;
	}

	// Le bloc de données est récupéré, modifié puis réécrit.
	char fileData[BLOCK_SIZE];
  ReadBlock(fileiNode.Block[0], fileData);
  for (int i = offset; i < (offset + numbytes); i++)
    fileData[i] = buffer[i-offset];
	WriteBlock(fileiNode.Block[0], fileData);

	// Les lignes suivantes déterminent la nouvelle taille du fichier.
	int newFileSize;
	if (offset + numbytes <= fileSize)
		newFileSize = fileSize;
	else
	{
		int newBytes = (offset + numbytes) - fileSize;
		newFileSize = fileSize + newBytes;
	}
	fileiNode.iNodeStat.st_size = newFileSize;
	writeInode(&fileiNode);

  return numbytes;
}

int bd_hardlink(const char *pPathExistant, const char *pPathNouveauLien) {
  char dirNewLink[FILENAME_SIZE];
  GetDirFromPath(pPathNouveauLien, dirNewLink);

  ino existingIno = getInodeFromPath(pPathExistant);
  ino newLinkIno = getInodeFromPath(dirNewLink);
  iNodeEntry existingIE, newLinkIE;

  if(existingIno == -1 || newLinkIno == -1) {
    return -1; //Un des fichiers est inexistant.
  }

  if(getIE(existingIno, &existingIE) != 0) {
    return -1; //pPathExisant n'existe pas.
  }

  if(getIE(newLinkIno, &newLinkIE) != 0) {
    return -1; //pPathNouveauLien n'existe pas.
  }

  if(getInodeFromPath(pPathNouveauLien) != -1) {
    return -2; //Le fichier nouveauLien existe déjà.
  }

  if(existingIE.iNodeStat.st_mode & G_IFDIR) {
    return -3; //C'est un répertoire.
  }

  char data[BLOCK_SIZE], newLinkName[FILENAME_SIZE];
  GetFilenameFromPath(pPathNouveauLien, newLinkName);
  
  ReadBlock(newLinkIE.Block[0], data);
  DirEntry *pEntries = (DirEntry *) data;
  int entryNumber = NumberofDirEntry(newLinkIE.iNodeStat.st_size);
  pEntries[entryNumber].iNode = existingIE.iNodeStat.st_ino; //On assigne l'inode du fichier existant à la fin des entries.
  strcpy(pEntries[entryNumber].Filename, newLinkName); //Le nom du fichier aussi.
  newLinkIE.iNodeStat.st_size += sizeof(DirEntry);
  existingIE.iNodeStat.st_nlink++;

  writeInode(&existingIE);
  writeInode(&newLinkIE);
  WriteBlock(newLinkIE.Block[0], data);
  
  return 0;
}

int bd_unlink(const char *pFilename) {
  char dName[BLOCK_SIZE], fName[FILENAME_SIZE];
  GetDirFromPath(pFilename, dName);
  GetFilenameFromPath(pFilename, fName);

  ino dIno = getInodeFromPath(dName);
  ino fIno = getInodeFromPath(pFilename);
  iNodeEntry dIE, fIE;

  if(dIno == -1 || fIno == -1) {
    return -1; //Fichier ou repertoire non existant.
  }

  if(getIE(dIno, &dIE) != 0) {
    return -1; //Le repertoire n'existe pas.
  }

  if(getIE(fIno, &fIE) != 0) {
    return -1; //Le fichier n'existe pas.
  }

  if(fIE.iNodeStat.st_mode & G_IFDIR) {
    return -2; //Le fichier est un répertoire.
  }

  char data[BLOCK_SIZE];
  ReadBlock(dIE.Block[0], data);
  DirEntry *pDE = (DirEntry *) data;
  int entryNumber = NumberofDirEntry(dIE.iNodeStat.st_size);
  int entry, offset;
  for(entry = 0; entry < entryNumber; entry++){
    if(strcmp(pDE[entry].Filename, fName) == 0) {
      if(entry != entryNumber - 1)//S'il n'est pas déjà dernier, on mets à jour la table DirEntry en décalant chacun des éléments stocké plus loin
      {
	for(offset =1; offset < entryNumber - entry; offset++){
	  pDE[entry + (offset - 1)] = pDE[entry + offset];
	}
	break;
      }
    }
  }
  WriteBlock(dIE.Block[0], data); //Enregistre le compactage

  fIE.iNodeStat.st_nlink--; //Décrémente nLink
  if(fIE.iNodeStat.st_nlink == 0){
    if(fIE.iNodeStat.st_blocks > 0){
      releaseBlock(fIE.Block[0]); //Si le nombre de lien est à 0, on lache les inode et bloque du fichier.
    }
    releaseInode(fIno);
  }
  else{
    writeInode(&fIE);
  }

  dIE.iNodeStat.st_size -= sizeof(DirEntry);
  writeInode(&dIE);
  
  return 0;
}

int bd_truncate(const char *pFilename, int NewSize) {
  ino iNodeNumber = getInodeFromPath(pFilename);
  if (iNodeNumber == -1)
    return -1; // Le fichier n'existe pas.

  iNodeEntry fileiNode;
  if(getIE(iNodeNumber, &fileiNode) != 0)
    return -1;

  if (fileiNode.iNodeStat.st_mode & G_IFDIR)
    return -2; // Ce n'est pas un fichier, mais un répertoire.

  char data[BLOCK_SIZE];
  int bytes;
  int currentSize = fileiNode.iNodeStat.st_size;
  if (NewSize > currentSize ) {
    return currentSize;
  }
  else if( NewSize == 0){
    releaseBlock(fileiNode.Block[0]);
    fileiNode.iNodeStat.st_size = 0;
    fileiNode.iNodeStat.st_blocks = 0;
    writeInode(&fileiNode);
    return 0;
  }
  else {
    for(bytes = currentSize; bytes > (currentSize - NewSize); bytes--) {
      data[bytes] = 0;
    }
    fileiNode.iNodeStat.st_size = NewSize;
    writeInode(&fileiNode);
  }
  return NewSize;
}

int bd_rmdir(const char *pFilename) {
  char strSubDir[BLOCK_SIZE];
  char strFilename[FILENAME_SIZE];
  ino subDirIno, dirNameIno;
  
  if(GetDirFromPath(pFilename, strSubDir) == 0) {
    return -1; //pDirName ne contient aucun /.
  }
  if(GetFilenameFromPath(pFilename, strFilename) == 0) {
    return -1; //Invalide.
  }
  subDirIno = getInodeFromPath(strSubDir);
  dirNameIno = getInodeFromPath(pFilename);
  if(subDirIno == -1 || dirNameIno == -1) {
    return -1; //N'existe pas.
  }
  iNodeEntry pInodeSubDir, pInodeDir;
  if(getIE(subDirIno,&pInodeSubDir) == -1){
    return -1; //Le répertoire parent est non valide
  }
  if(getIE(dirNameIno,&pInodeDir) == -1) {
    return -1; //Le répertoire est non valide
  }
  if(pInodeDir.iNodeStat.st_mode & G_IFREG) {
    return -2; //Fichier régulier.
  }
  UINT16 nentries = NumberofDirEntry(pInodeDir.iNodeStat.st_size);
  if(nentries  > 2){
    return -3; //Pas vide.
  }
  removeDir(&pInodeSubDir, dirNameIno);
  pInodeSubDir.iNodeStat.st_nlink--;
  writeInode(&pInodeSubDir);
  releaseBlock(pInodeDir.Block[0]);
  releaseInode(dirNameIno);
  return 0;
}

int bd_rename(const char *pFilename, const char *pDestFilename) {
	// Si les répertoires sont identiques, ne rien faire.
	if(pFilename == pDestFilename)
		return 0;

	int hardlink = bd_hardlink(pFilename, pDestFilename);

	// Si hardlink est réussi, seulement unlink la source.
	if (hardlink == 0)
		return bd_unlink(pFilename);

	// Si hardlink échoue, alors pFilename est invalide ou le directory de pDestFilename est invalide.
	else if (hardlink == -2 || hardlink == -1)
		return -1;
	
	else { // Si la source est un répertoire
		
		char sourceDir[BLOCK_SIZE];
		char targetDir[BLOCK_SIZE];
		char targetFilename[FILENAME_SIZE];

		// On récupere les paths du sourceDirectory, targetDirectory et le filename de la source.
		GetDirFromPath(pFilename, sourceDir);
		GetDirFromPath(pDestFilename, targetDir);
		GetFilenameFromPath(pDestFilename, targetFilename);

		// On récupere les numeros d'inode du sourceDirectory, targetDirectory et de la source.
		ino sourceDirectoryInodeNumber = getInodeFromPath(sourceDir);
		ino targetDirectoryInodeNumber = getInodeFromPath(targetDir);
		ino sourceInodeNumber = getInodeFromPath(pFilename);

		iNodeEntry sourceDirectoryInode;
		iNodeEntry targetDirectoryInode;
		iNodeEntry sourceInode;

		// On récupere les inodes avec les numeros d'inode précédemment récupérés.
		getIE(sourceDirectoryInodeNumber, &sourceDirectoryInode);
		getIE(targetDirectoryInodeNumber, &targetDirectoryInode);
		getIE(sourceInodeNumber, &sourceInode);

		// Si le sourceDir et le targetDir est le meme, on ne fait que rename la source.
		if (sourceDirectoryInodeNumber == targetDirectoryInodeNumber) {
			removeDir(&sourceDirectoryInode, sourceInodeNumber);
			addDir(&sourceDirectoryInode, targetFilename, sourceInodeNumber);
			writeInode(&sourceDirectoryInode);
			return 0;
		}

		// On commence par retirer les liens de la source à son parent
		removeDir(&sourceDirectoryInode, sourceInodeNumber);
		sourceDirectoryInode.iNodeStat.st_nlink--;
		writeInode(&sourceDirectoryInode);

		// On ajoute au targetDir la source, avec le nouveau filename.
		addDir(&targetDirectoryInode, targetFilename, sourceInodeNumber);
		targetDirectoryInode.iNodeStat.st_nlink++;
		writeInode(&targetDirectoryInode);

		return 0;
	}
}

int bd_readdir(const char *pDirLocation, DirEntry **ppListeFichiers) {
  ino iNodeNumber = getInodeFromPath(pDirLocation);
  if (iNodeNumber == -1)
    return -1; //Directory inexistant.

  iNodeEntry diriNode;
	getIE(iNodeNumber, &diriNode);

  if (!(diriNode.iNodeStat.st_mode & G_IFDIR))
    return -1; // Le fichier spécifié n'est pas un directory.
  
  char dirEntries[BLOCK_SIZE];
  ReadBlock(diriNode.Block[0], dirEntries);
  int dirSize = diriNode.iNodeStat.st_size;

  *ppListeFichiers = (DirEntry*) malloc(dirSize);
	memcpy((*ppListeFichiers), dirEntries, dirSize);

  return NumberofDirEntry(dirSize);
}

int bd_symlink(const char *pPathExistant, const char *pPathNouveauLien) {
  char destDir[BLOCK_SIZE];
  char newLinkDir[BLOCK_SIZE];
  ino nlIno, fIno = getInodeFromPath(pPathExistant);
  iNodeEntry newLinkIno;

  if(GetDirFromPath(pPathNouveauLien, destDir) == 0){
    return -1; //Répertoire inexistant.
  }

  if(GetFilenameFromPath(pPathNouveauLien, newLinkDir) ==0){
    return -1; //Fichier inexistant
  }

  nlIno = getInodeFromPath(pPathNouveauLien);
  if(getIE(nlIno, &newLinkIno) == 0) {
    return -2; //Existe déjà
  }

  //ça passe, on peut créer le fichier
  bd_create(pPathNouveauLien);
  nlIno = getInodeFromPath(pPathNouveauLien);
  if(getIE(nlIno, &newLinkIno) != 0){
    return -1; //Si plus de place
  }

  newLinkIno.iNodeStat.st_mode |= G_IFLNK | G_IFREG;
  writeInode(&newLinkIno);
  bd_write(pPathNouveauLien, pPathExistant, 0, strlen(pPathExistant)+1); //Écriture du path exisant dans le lien
    
  return 0;
}

int bd_readlink(const char *pPathLien, char *pBuffer, int sizeBuffer) {
  ino nIno = getInodeFromPath(pPathLien);
  iNodeEntry pIE;

  if(nIno == -1) return -1;
  if(getIE(nIno, &pIE) != 0) return -1; //N'existe pas 
  if(!(pIE.iNodeStat.st_mode & G_IFREG) || !(pIE.iNodeStat.st_mode & G_IFLNK)) return -1;// N'est pas un lien symbolique

  char data[BLOCK_SIZE];
  ReadBlock(pIE.Block[0], data);
  int i;
  for(i=0; i < pIE.iNodeStat.st_size && i < sizeBuffer; i++){
    pBuffer[i] = data[i]; //On stock ce qui est écrit dans le fichier
  }
  return i;
}
