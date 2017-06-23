#include "BST.h"

void create_booktree() {
    root=NULL;
    atribute=1;
}

void delete_booktree(NodeBT* start) {
    if(start!=NULL) {
        delete_booktree(start->left);
        delete_booktree(start->right);
        destroy_person(start->person);
        free(start);
    }
}

int compare(int i, Person *p1, Person *p2) {
    if(i==1) return strcmp(p1->last_name, p2->last_name);
    else if(i==2) return strcmp(p1->birth_date, p2->birth_date);
    else if(i==3) return strcmp(p1->email, p2->email);
    else return strcmp(p1->phone, p2->phone);

}

void add_contact_tree(Person *p, NodeBT *start) {
    if (root == NULL) {
        root = malloc(sizeof (*root));
        root->person = p;
        root->left = NULL;
        root->right = NULL;
        root->parent= NULL;
    }
    else if(compare(atribute, p, start->person)<0) {
        if(start->left != NULL) {
            add_contact_tree(p, start->left);
        }
        else {
            NodeBT *node = malloc(sizeof(*node));
            node->person = p;
            node->left = NULL;
            node->right = NULL;
            node->parent = start;
            start->left=node;
        }
    }
    else {
        if(start->right!=NULL)
        {
            add_contact_tree(p, start->right);
        }
        else
        {
            NodeBT *node = malloc(sizeof (*node));
            node->person = p;
            node->left = NULL;
            node->right = NULL;
            node->parent = start;
            start->right=node;
        }
    }
}

NodeBT* themost_left(NodeBT *start) {
    if(start->left != NULL)
        return themost_left(start->left);
    else
        return start;
}

NodeBT* find_contact_tree(NodeBT *start, Person *p) {
    if (compare(atribute, p, start->person)==0)
        return start;
    else if (compare(atribute, p, start->person)<0 && start->left != NULL)
        return find_contact_tree(start->left, p);
    else if (compare(atribute, p, start->person)>0 && start->right != NULL)
        return find_contact_tree(start->right, p);
    return NULL;
}

NodeBT* delete_contact_from_book(NodeBT *start) {
    //jezeli wezel nie ma dzieci
    if(start->left==NULL && start->right==NULL) {
        //jezeli wezel jest korzeniem
        if(start->parent==NULL){
            root=NULL;
        }
        //jezeli wezel jest po lewej stronie parenta,
        else if(start->parent->left==start) {
            //usun wezel z lewej strony wezla parenta
            start->parent->left=NULL;
        }
        else{
            //usun wezel z prawej strony wezla parenta
            start->parent->right=NULL;
        }
        return start;
    }
    //jezeli wezel ma tylko jedno dziecko
    else if(start->left==NULL || start->right==NULL) {
        //jezeli po lewej stronie nie ma dziecka
        if(start->left==NULL) {
            //jezeli wezel jest korzeniem
            if(start->parent==NULL) {
                root=start->right;
            }
            //jezeli wezel jest po lewej stronie parenta
            else if(start->parent->left==start) {
                //przyczep z lewej strony parenta wezel bedacy po prawej stronie usuwanego wezla
                start->parent->left=start->right;
            }
            else{
                //przyczep z prawej strony parenta wezel bedacy po prawej stronie usuwanego wezla
                start->parent->right=start->right;
            }
            start->right->parent=start->parent;
        }
        else {
            //jezeli wezel jest korzeniem
            if(start->parent==NULL) {
                root=start->left;
            }
            //jezeli wezel jest po lewej stronie parenta
            else if(start->parent->left==start) {
                //przyczep z lewej strony parenta wezel bedacy po lewej stronie usuwanego wezla
                start->parent->left=start->left;
            }
            else{
                //przyczep z prawej strony parenta wezel bedacy po prawej stronie usuwanego wezla
                start->parent->right=start->left;
            }
            start->left->parent=start->parent;
        }
        return start;
    }
    //jezlei wezel ma dwojke dzieci
    else {
        //wstaw w miejsce usuwanego elementu - najmniejsza p z prawego poddrzewa
        NodeBT *tmp=themost_left(start->right);
        Person *ps=start->person;
        start->person = tmp->person;
        tmp->person=ps;
        return delete_contact_from_book(tmp);
    }
}

void delete_contact_tree(Person *p) {
    NodeBT* node=find_contact_tree(root, p);
    NodeBT* tmp=delete_contact_from_book(node);
    destroy_person(tmp->person);
    free(tmp);
}

void add_contact_wrebuilding(NodeBT *tmp, NodeBT *start, int which_atribute) {
    if (root_rebuilt == NULL) {
        root_rebuilt = tmp;
        root_rebuilt->left = NULL;
        root_rebuilt->right = NULL;
        root_rebuilt->parent= NULL;
    }
    else if (compare(which_atribute, tmp->person, start->person)<0) {
        if(start->left != NULL) {
            add_contact_wrebuilding(tmp, start->left, which_atribute);
        }
        else {
            start->left=tmp;
            tmp->left = NULL;
            tmp->right = NULL;
            tmp->parent = start;
        }
    }
    else {
        if(start->right!=NULL)
        {
            add_contact_wrebuilding(tmp, start->right, which_atribute);
        }
        else {
            start->right=tmp;
            tmp->left = NULL;
            tmp->right = NULL;
            tmp->parent = start;
        }
    }
}

void build_newtree(NodeBT* start, int which_atribute) {
    if(start!=NULL) {
        build_newtree(start->left, which_atribute);
        build_newtree(start->right, which_atribute);
        NodeBT* tmp=delete_contact_from_book(start);
        add_contact_wrebuilding(tmp, root_rebuilt, which_atribute);
    }
}

void rebuild_tree(NodeBT* start, int which_atribute) {
    build_newtree(start, which_atribute);
    atribute=which_atribute;
    root=root_rebuilt;
    root_rebuilt=NULL;
}

void print_tree(NodeBT *root){
    if(root!=NULL) {
        print_person(*(root->person));
        print_tree(root->left);
        print_tree(root->right);
    }
    else{
        printf("NULL ");
    }
}
