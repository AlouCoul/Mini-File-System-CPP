/**
 * \file block.cpp
 * \brief Implémentation d'un bloc.
 * \author ?
 * \version 0.1
 * \date  2024
 *
 *  Travail pratique numéro 3
 *
 */

#include "block.h"
//vous pouvez inclure d'autres librairies si c'est nécessaire

namespace TP3 {

    Block::Block() {
        // Initialisation des membres de la classe
        m_type_donnees = 0;
        m_inode = nullptr;
    }

    Block::Block(size_t td) : m_type_donnees(td) {
        // Initialisation des autres membres de la classe
        m_type_donnees = td;
        m_inode = nullptr;
    }

    Block::~Block() {
        // Libération des ressources, si nécessaire
        delete m_inode;
    }

} // namespace TP3
