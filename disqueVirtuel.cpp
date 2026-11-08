/**
 * \file disqueVirtuel.cpp
 * \brief Implémentation d'un disque virtuel.
 * \author ?
 * \version 0.1
 * \date  2024
 *
 *  Travail pratique numéro 3
 *
 */

#include "disqueVirtuel.h"
#include <iostream>
#include <string>

//vous pouvez inclure d'autres librairies si c'est nécessaire
#include <vector>
#include <sstream>
#include <iterator>

using namespace std;

namespace TP3
{
	DisqueVirtuel::DisqueVirtuel() {
        // Initialisation du disque virtuel avec N_BLOCK_ON_DISK blocs
        m_blockDisque = vector<Block>(N_BLOCK_ON_DISK);
    }

    DisqueVirtuel::~DisqueVirtuel() {
        // Pas d'actions spécifiques requises pour la destruction de l'objet
    }

	int DisqueVirtuel::bd_FormatDisk() {
		// Efface et initialise le disque virtuel
		m_blockDisque.clear();
        m_blockDisque = vector<Block>(N_BLOCK_ON_DISK);

		// Initialise la bitmap des blocs libres
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap = vector<bool>(N_BLOCK_ON_DISK);
		for (int i = 0; i < N_BLOCK_ON_DISK; i++) {
			m_blockDisque[2].m_bitmap[i] = true;
		}
		
		// Initialise le bloc de bitmap des inodes
		m_blockDisque[FREE_INODE_BITMAP] = *new Block(S_IFIL);
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap = vector<bool>(N_INODE_ON_DISK);

		// Initialise les blocs de données et métadonnées
		for (int i = 0; i < N_BLOCK_ON_DISK; i++) {
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = true;
            m_blockDisque[i] = *new Block();
		}

		// Initialise les inodes et leur bitmap
		for (int i = 0; i < N_INODE_ON_DISK; i++) {
			m_blockDisque[i + BASE_BLOCK_INODE] = *new Block(S_IFIN);
			m_blockDisque[i + BASE_BLOCK_INODE].m_inode = new iNode(i, 0, 0, 0, 0);
			m_blockDisque[i + BASE_BLOCK_INODE].m_type_donnees = S_IFIN;
			m_blockDisque[FREE_INODE_BITMAP].m_bitmap[i] = true;
			m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i + BASE_BLOCK_INODE] = false;
		}

        for (int i = 0; i < N_INODE_ON_DISK + BASE_BLOCK_INODE; i++) {
            m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i] = false;
        }

		// Premier inode utilisé
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[0] = false;

		// Création du répertoire racine
        int rootBlock = getFreeData();
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[rootBlock]= false;
        int rootInode = getFreeInode();
        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[rootInode - BASE_BLOCK_INODE] = false;
        
		m_blockDisque[rootInode].m_inode = new iNode(1, S_IFDIR, 2, 0, rootBlock);
        m_blockDisque[rootInode].m_inode->st_size = 56;
		m_blockDisque[rootBlock].m_dirEntry.push_back(new dirEntry(rootInode, "."));
		m_blockDisque[rootBlock].m_dirEntry.push_back(new dirEntry(rootInode, ".."));

		cout << "UFS: saisir i-node " << rootInode - BASE_BLOCK_INODE << endl;
        cout << "UFS: saisir bloc " << rootBlock << endl;

		return 1; // Succès
	}

    int DisqueVirtuel::bd_create(const string& p_FileName) {
		vector<string> path = splitPath(p_FileName);

		int inode = findInode(vector<string>(path.begin(), path.begin() + (path.size() -1)));
		if (inode == 0) return 0;
		return createFile(inode, path[path.size()-1]);
	}

    int DisqueVirtuel::bd_mkdir(const string& p_DirName) {
        vector<string> path = splitPath(p_DirName);

		int currentInode = findInode(vector<string>(path.begin(), path.begin() + (path.size() - 1)));
		if (currentInode == 0) return 0;
		return createDirectory(currentInode, path[path.size()-1]);
    }

	string DisqueVirtuel::bd_ls(const string& p_DirLocation) {
		// Trouve l'i-node correspondant au répertoire spécifié par le chemin
		vector<string> path = splitPath(p_DirLocation);
		int inode = findInode(path);
		if (inode == 0) return "";

		// Récupère le bloc d'entrées de répertoire du répertoire
		const auto& dirEntryBlock = (m_blockDisque[inode].m_inode->st_mode == S_IFDIR) ? m_blockDisque[m_blockDisque[inode].m_inode->st_block] : Block();
		// Initialise la chaîne de retour avec le chemin
		string output = p_DirLocation + "\n";

		for (const auto& entry : dirEntryBlock.m_dirEntry) {
			const iNode* tempInode = m_blockDisque[entry->m_iNode].m_inode;
			string type = (tempInode->st_mode == S_IFDIR) ? "d" : "-";
			string filename = entry->m_filename;
			filename += (filename.length() < 8) ? "\t" : "\t\t";
			output += type + "\t" + filename + " Size:\t" + to_string(tempInode->st_size) + " inode:\t" + to_string(tempInode->st_ino) + " nlink:\t" + to_string(tempInode->st_nlink) + "\n";
		}

		return output;
	}

    int DisqueVirtuel::bd_rm(const std::string& p_Filename) {
		vector<string> path = splitPath(p_Filename);

		// Trouve linode du répertoire parent et du fichier à supprimer
        int parentInode = findInode(vector<string>(path.begin(), path.begin() + (path.size() - 1)));
        int inode = findFile(parentInode, path[path.size() - 1]);
        if (inode == -1) return 0;

        string blocString = "";
		// Vérifie si le fichier est un dossier et s'il n'est pas vide
        if (m_blockDisque[inode].m_inode->st_mode == S_IFDIR) {
            if (m_blockDisque[m_blockDisque[inode].m_inode->st_block].m_dirEntry.size() > 2) {
                return 0;
            }
            // Suppression du bloc de données du répertoire
            m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[m_blockDisque[inode].m_inode->st_block] = true;
            blocString = "UFS: Relache bloc " + to_string(m_blockDisque[inode].m_inode->st_block) + "\n";
        }

        // Suppression de l'entrée dans le repertoire parent
        vector<dirEntry*> entries = m_blockDisque[m_blockDisque[parentInode].m_inode->st_block].m_dirEntry;

        for (int i = 0; i < entries.size(); i++) {
            if (entries[i]->m_filename == path[path.size() - 1]) {
                m_blockDisque[m_blockDisque[parentInode].m_inode->st_block].m_dirEntry.erase(m_blockDisque[m_blockDisque[parentInode].m_inode->st_block].m_dirEntry.begin() + i);
                break;
            }
        }

         m_blockDisque[parentInode].m_inode->st_nlink--;
         m_blockDisque[parentInode].m_inode->st_size -= 28;

        m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inode - BASE_BLOCK_INODE] = true;
        m_blockDisque[inode].m_inode->st_nlink--;

        if (m_blockDisque[inode].m_inode->st_nlink <= 0) {
            delete m_blockDisque[inode].m_inode;
            m_blockDisque[inode].m_inode = nullptr;
        }

        cout << "UFS: Relache i-node " << inode - BASE_BLOCK_INODE << endl;
        cout << blocString;

		return 1; // Succès
	}

    int DisqueVirtuel::getFreeData() {
		// Parcours du bitmap des blocs libres
		for (int i = 0; i < N_BLOCK_ON_DISK; i++) {
			if (m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[i]) {
				// Retourne l'index du premier bloc libre
				return i;
			}
		}
		return -1;
	}

    int DisqueVirtuel::getFreeInode() {
		// Parcours du bitmap des inodes libres
		for (int i = 0; i < N_INODE_ON_DISK; i++) {
			if (m_blockDisque[FREE_INODE_BITMAP].m_bitmap[i]) {
				// Retourne l'index ajusté du premier inode libre
				return i + BASE_BLOCK_INODE;
			}
		}
		return -1;
	}

    vector<string> DisqueVirtuel::splitPath(const string& p_path) {
		// Initialise le chemin et le vecteur
		string path = p_path.substr(1);
		vector<string> pathComponents = { "." };

		// Boucle pour séparer le chemin en sous-chemins
		while (!path.empty()) {
			size_t index = path.find_first_of("/");

			if (index == string::npos) {
				pathComponents.push_back(path);
				break;
			}

			// Ajoute le sous-chemin au vecteur
			pathComponents.push_back(path.substr(0, index));
			path = path.substr(index + 1);
		}

		return pathComponents;
	}

    int DisqueVirtuel::findInode(vector<string> path) {
		int inode = BASE_BLOCK_INODE + ROOT_INODE;

		for (const string& folderName : path) {
			// Récupération du bloc du répertoire actuel
			int block = m_blockDisque[inode].m_inode->st_block;

			// Recherche de l'entrée correspondant au nom de dossier dans le bloc
			bool found = false;
			for (const auto& entry : m_blockDisque[block].m_dirEntry) {
				if (entry->m_filename == folderName) {
					inode = entry->m_iNode;
					found = true;
					break;
				}
			}
			// Si le dossier n'existe pas ou n'est pas un dossier
			if (!found || m_blockDisque[inode].m_inode->st_mode != S_IFDIR) {
				return 0;
			}
		}
		return inode;
	}

    int DisqueVirtuel::createFile(int parentInode, const string& fileName) {
		// Recherche d'un bloc inode libre
		int inode = getFreeInode();
		if (inode == -1) {
			return 0;
		}

		// Création du nouvel inode
		iNode* newInode = new iNode(inode, S_IFREG, 1, 0, 0);
		m_blockDisque[inode].m_inode = newInode;

		// Ajout de l'entrée de répertoire dans le répertoire parent
		m_blockDisque[m_blockDisque[parentInode].m_inode->st_block].m_dirEntry.push_back(new dirEntry(inode, fileName));
		m_blockDisque[parentInode].m_inode->st_size += 28;

		// Marquage du bloc inode comme utilisé dans le bitmap des inodes libres
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inode - BASE_BLOCK_INODE] = false;

		cout << "UFS: saisir i-node " << inode - BASE_BLOCK_INODE << endl;
		
		return 1;
	}

	int DisqueVirtuel::createDirectory(int parentInode, const string& directoryName) {
		// Recherche d'un bloc inode et d'un bloc de données libre
		int inode = getFreeInode();
		int data = getFreeData();
		if (inode == -1 || data == -1) {
			return 0;
		}

		// Ajout du lien dans le dossier parent
		m_blockDisque[m_blockDisque[parentInode].m_inode->st_block].m_dirEntry.push_back(new dirEntry(inode, directoryName));
		m_blockDisque[parentInode].m_inode->st_nlink++;

		// Ajout des liens dans le nouveau dossier
		m_blockDisque[data].m_dirEntry.push_back(new dirEntry(parentInode, ".."));
		m_blockDisque[data].m_dirEntry.push_back(new dirEntry(inode, "."));

		// Initialisation du nouvel i-node pour le dossier
		m_blockDisque[inode].m_inode = new iNode(inode - BASE_BLOCK_INODE, S_IFDIR, 2, 56, data);
		m_blockDisque[parentInode].m_inode->st_size += 28;

		// Marquage du nouvel i-node et du bloc de données comme utilisés
		cout << "UFS: saisir i-node " << inode - BASE_BLOCK_INODE << endl;
		m_blockDisque[FREE_INODE_BITMAP].m_bitmap[inode - BASE_BLOCK_INODE] = false;
		cout << "UFS: saisir bloc " << data << endl;
		m_blockDisque[FREE_BLOCK_BITMAP].m_bitmap[data] = false;

		return 1;
	}

	int DisqueVirtuel::findFile(int parentInode, const string& fileName){
        int data = m_blockDisque[parentInode].m_inode->st_block;

		// Parcourt les entrées du répertoire parent
        for (int j = 0; j < m_blockDisque[data].m_dirEntry.size(); j++) {
            if (m_blockDisque[data].m_dirEntry[j]->m_filename == fileName)
            {
				// Retourne linode du fichier
                return m_blockDisque[data].m_dirEntry[j]->m_iNode;
            }
        }
        return -1;
    }
}
