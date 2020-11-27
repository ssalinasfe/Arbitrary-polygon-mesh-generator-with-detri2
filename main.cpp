#include <iostream>
#include <stdio.h>   
#include <stdlib.h>     /* exit, EXIT_FAILURE */
#include "detri2.h"
#include "polymesh.h"
#include <vector> 

#include "io.h"
#include "consts.h"
#include "triang.h"
#include "polygon.h"


#ifdef DEBUG
#define DEBUG_TEST 1
#else
#define DEBUG_TEST 0
#endif

#define debug_block(fmt) do { if (DEBUG_TEST){ fmt }} while (0)
#define debug_print(fmt, ...) do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, __LINE__, __func__, __VA_ARGS__); } while (0)
#define debug_msg(fmt) do { if (DEBUG_TEST) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__,  __LINE__, __func__); } while (0)


int main(int argc, char* argv[]){


    int nparam = 3;
    //char* params[] = {const_cast<char*> ("./detri2"), const_cast<char*> ("-z"), const_cast<char*> ("test.node")};
	char* params[] = {const_cast<char*> ("./detri2"), const_cast<char*> ("-z"), const_cast<char*> ("506randompoints.node")};
	//char* params[] = {const_cast<char*> ("./detri2"), const_cast<char*> ("-z"), const_cast<char*> ("506equilateral.node")};
    int print_triangles = 0;
    char* ppath;
    //char* ppath = const_cast<char*> ("test");
    TMesh *Tr = new TMesh(nparam, params);    
    Tr->print();
    
	int tnumber, pnumber, i,j;
	double *r;
	int *triangles;
	int *adj;
    int *visited;
    int *max;
	std::vector<int> border_point;

    tnumber = Tr->tnumber;
    pnumber = Tr->pnumber;

    max = (int *)malloc(tnumber*sizeof(int));
	visited = (int *)malloc(tnumber*sizeof(int));
    r = (double *)malloc(2*tnumber*sizeof(double));
    adj =(int *)malloc(3*tnumber*sizeof(int));
    triangles =   (int *)malloc(3*tnumber*sizeof(int));

    int idx =0;
    //copiar arreglo de vertices
    std::cout<<"pnumber "<<pnumber<<std::endl;
    for (i = 0; i < Tr->trimesh->ct_in_vrts; i++) {
        if (!Tr->trimesh->io_keep_unused) { // no -IJ
            if (Tr->trimesh->in_vrts[i].typ == UNUSEDVERTEX) continue;
        }
        r[2*i + 0]= Tr->trimesh->in_vrts[i].crd[0];
        r[2*i + 1]= Tr->trimesh->in_vrts[i].crd[1];
        std::cout<<idx<<" ("<<r[2*i + 0]<<", "<<r[2*i + 1]<<") "<<std::endl;
        Tr->trimesh->in_vrts[i].idx = idx;
        idx++;
    }
    idx = 0;
    for (int i = 0; i < Tr->trimesh->tr_tris->used_items; i++) {
        detri2::Triang* tri = (detri2::Triang *) Tr->trimesh->tr_tris->get(i);
        if (tri->is_deleted()) continue;
        if (tri->is_hulltri()) {
            tri->idx = -1;
        } else {
            tri->idx = idx;
            idx++;
        }
    }

    std::cout<<"tnumber: "<<Tr->trimesh->tr_tris->objects - Tr->trimesh->ct_hullsize<<std::endl;
    idx = 0;
    for (int i = 0; i < Tr->trimesh->tr_tris->used_items; i++)
    {
        
        detri2::Triang* tri = (detri2::Triang *) Tr->trimesh->tr_tris->get(i);
        if (tri->is_deleted() || tri->is_hulltri()) continue;
        triangles[3*idx+0] = tri->vrt[0]->idx;
        triangles[3*idx+1] = tri->vrt[1]->idx;
        triangles[3*idx+2] = tri->vrt[2]->idx;
        adj[3*idx+ 0] = tri->nei[0].tri->idx;
        adj[3*idx+ 1] = tri->nei[1].tri->idx;
        adj[3*idx+ 2] = tri->nei[2].tri->idx;
        std::cout<<idx<<" | "<<triangles[3*idx+0]<<" "<<triangles[3*idx+1]<<" "<<triangles[3*idx+2]<<" | ";
        std::cout<<adj[3*idx+ 0]<<" "<<adj[3*idx+ 1]<<" "<<adj[3*idx+ 2]<<" | "<<std::endl;
        idx++;
    }

	std::cout<<tnumber<<" "<<pnumber<<" "<<std::endl;
	/* Etapa 1: Encontrar aristas máximas. */
	debug_msg("Etapa 1: Encontrar aristas máximas. \n");
	for(i = 0; i < tnumber; i++)
	{
		max[i] = max_edge_index(i, r, triangles);
	}
	
	/* Etapa 2: Desconectar arcos asociados a aristas nomáx-nomáx. */
	debug_msg("Etapa 2: Desconectar arcos asociados a aristas nomáx-nomáx. \n");
	for(i = 0; i < tnumber; i++)
	{
		for(j = 0; j < 3; j++)
		{
			if(adj[3*i +j] < 0){
				border_point.push_back(triangles[3*i + (j+1 %3)]);
				border_point.push_back(triangles[3*i + (j+2 %3)]);
			}
			if(adj[3*i +j] < 0 || is_nomax_nomax(i, adj[3*i + j], triangles, max))
			{
				adj[3*i + j] = NO_ADJ;
			}
			
		}
	}

	//debug_block(print_poly(root_id, tnumber););


	/* Se crean los poligonos */

	
	int *poly = (int *)malloc(tnumber*sizeof(int));
	int length_poly = 0;
	int *pos_poly = (int *)malloc(tnumber*sizeof(int));
	int id_pos_poly = 0;

	int *mesh;
	mesh = (int *)malloc(3*tnumber*sizeof(int));
	int i_mesh = 0;	
	
	int *chose_seed_triangle;
	chose_seed_triangle = (int *)malloc(tnumber*sizeof(int));
	int id_chose_seed_triangle = 0;
	int num_BE = 0;

	for(i = 0; i < tnumber; i++){
		visited[i] = FALSE;
	}

	debug_msg("Etapa 5: Generar poligonos\n");
	for(i = 0; i < tnumber; i++)
	{
		/*busca fronter edge en un triangulo, hacer función está wea*/
		
		

		/* si tiene 2-3 froint edge y no ha sido visitado*/
		//AGREGAR LA CONDICION DE QUE SI ROOT_ID = -1 ENTONCES NO GENERA POLY, MARCAR_ID[ALGO] == -1 DESPUES DE GENERAR C/POLY!!!
		int num_FrontierEdges = count_FrontierEdges(i, adj);
		if(!visited[i] && num_FrontierEdges > 0){

			chose_seed_triangle[id_chose_seed_triangle] = i;
			id_chose_seed_triangle++;

			length_poly = generate_polygon(poly, triangles, adj, r, visited, i);
			num_BE = count_BarrierEdges(poly, length_poly);
			
			//save_to_mesh(mesh, poly, &i_mesh, length_poly, pos_poly, &id_pos_poly);	
			
			if(num_BE>0){
				printf("%d %d\n", num_BE, length_poly);
			}

			debug_msg("Poly: "); debug_block(print_poly(poly, length_poly););
			if( num_BE > 0){
				debug_print("Se dectecto %d BE\n", num_BE);
				remove_BarrierEdge(poly, length_poly, num_BE, triangles, adj, r, tnumber, mesh, &i_mesh, pos_poly, &id_pos_poly, visited);
			}else{
				debug_msg("Guardando poly\n");
				save_to_mesh(mesh, poly, &i_mesh, length_poly, pos_poly, &id_pos_poly);	
			}
		}
	}
	
	
	
	for (i = 0; i < tnumber; i++) 
	{
		if(visited[i] == FALSE){
			fprintf(stderr,"ERROR hmnito mira, el triangulo %d no se visito ", i);
		}
	}
	
	
	write_geomview(r,triangles, pnumber, tnumber,i_mesh, mesh, id_pos_poly, pos_poly, print_triangles, ppath, chose_seed_triangle, id_chose_seed_triangle, border_point);


	free(r);
	free(triangles);
	free(adj);
	free(max );
	free(visited );
	free(chose_seed_triangle);
	free(mesh );
	free(poly);
	free(pos_poly);
    delete Tr;
	return EXIT_SUCCESS;
}
    
