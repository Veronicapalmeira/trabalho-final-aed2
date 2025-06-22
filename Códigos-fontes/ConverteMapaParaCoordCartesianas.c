#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NODES 10000
#define MAX_WAYS  5000
#define MAX_WAY_NODES 100

#define MAX_VERTICES 10000
#define INFINITO 1000000000
#define PI 3.14159265358979323846

typedef struct {
    long long id_original;
    double lat;
    double lon;
    double x;
    double y;
    int id_interno; // de 0 a n-1
} Node;

typedef struct {
    int node_ids[MAX_WAY_NODES]; // índices internos dos nós
    int count;
} Way;

Node nodes[MAX_NODES];
int total_nodes = 0;

Way ways[MAX_WAYS];
int total_ways = 0;


// Parâmetros da zona UTM 23S
const double a = 6378137.0;            // Semi-eixo maior WGS84
const double f = 1.0 / 298.257223563;  // Achatamento
const double k0 = 0.9996;
//const double e2 = f * (2 - f);
//const double lon0 = -45.0 * PI / 180.0; // Meridiano central da zona 23
const double lon0_deg = -45.0; // longitude central da zona 23S


// Procura o índice interno de um id original
int get_node_index(long long id) {
    for (int i = 0; i < total_nodes; i++) {
        if (nodes[i].id_original == id)
            return nodes[i].id_interno;
    }
    return -1; // não encontrado
}

// Extrai valor de atributo entre aspas, ex: lat="..." ? retorna ...
void extract_attr(const char* line, const char* key, char* value) {
    char* p = strstr(line, key);
    if (p) {
        p = strchr(p, '\"');
        if (p) {
            p++;
            char* q = strchr(p, '\"');
            if (q) {
                strncpy(value, p, q - p);
                value[q - p] = '\0';
            }
        }
    }
}

char *Left(char *str, int n)
{
	int i;
	char *substr;
	//substr = (char*) malloc(sizeof(char) * (strlen(str)+1));
	
	if ((substr = (char*) malloc(sizeof(char) * (strlen(str)+1))) == NULL) {
		printf("Funcao Left: Memoria insuficiente para alocacao dinamica!\n");
		system("pause > nul");
		exit(1);  /* terminate program if out of memory */
	}	
	
	for (i=0; i<n; i++)
		substr[i] = str[i];
	
	substr[i] = '\0';
	return(substr);
}


// Retorna uma substring de determinado tamanho a partir de uma posiï¿½ï¿½o
char *Substr(char *s, int pos, int n)
{
  char *str;
  int i;

	if ((str = (char *) malloc(strlen(s)+1)) == NULL) {
    printf("Funcao Substr: Memoria insuficiente para alocacao dinamica!\n");
    system("pause > nul");
    exit(1);  /* terminate program if out of memory */
  }

  for (i=0; i<n; i++)
    str[i] = s[pos+i];

  str[i] = '\0';

  return str;
}

// Retorna a posicao da ultima ocorrencia de uma substring
int RAt(char *sub, char *string)
{
	int j;
 	int pos = -1;
 	int tamString, tamSub = strlen(sub);

 	if (strstr(string, sub) != NULL) {
		tamString = strlen(string);
  	for (j=tamString-1; j >= 0; j--) {
	 		if (stricmp(Substr(string, j, tamSub), sub) == 0) {
				pos = j;
    		break;
   		}
   		else
  			pos = -1;
		}
	}
 	return(pos);
}

// Retorna a posicao da primeira ocorrencia de uma substring
int At(char sub[], char string[])
{
	int j;
	int pos = -1;
	int tamString, tamSub = strlen(sub);
	
	if (strstr(string, sub) != NULL) {
		tamString = strlen(string);
	  for (j=0; j<tamString; j++) {
			if (strcmp(Substr(string, j, tamSub), sub) == 0) {
		    pos = j;
		    break;
		  }
		  else
		    pos = -1;
		}
	}
 	return(pos);
}


void converter_para_utm(double lat_deg, double lon_deg, double* x, double* y) {
    double e2 = f * (2 - f);                    // excentricidade ao quadrado
    double ep2 = e2 / (1 - e2);                 // excentricidade secundária ao quadrado
    double lat = lat_deg * PI / 180.0;
    double lon = lon_deg * PI / 180.0;
    double lon0 = lon0_deg * PI / 180.0;

    double N = a / sqrt(1 - e2 * sin(lat) * sin(lat));
    double T = tan(lat) * tan(lat);
    double C = ep2 * cos(lat) * cos(lat);
    double A = (lon - lon0) * cos(lat);

    // Cálculo do arco meridional com mais termos
    double M = a * ((1 - e2/4 - 3*e2*e2/64 - 5*e2*e2*e2/256) * lat
      - (3*e2/8 + 3*e2*e2/32 + 45*e2*e2*e2/1024) * sin(2*lat)
      + (15*e2*e2/256 + 45*e2*e2*e2/1024) * sin(4*lat)
      - (35*e2*e2*e2/3072) * sin(6*lat));

    // Coordenada leste (X)
    *x = k0 * N * (A + (1 - T + C) * pow(A,3)/6
         + (5 - 18*T + T*T + 72*C - 58*ep2) * pow(A,5)/120) + 500000.0;

    // Coordenada norte (Y)
    *y = k0 * (M + N * tan(lat) * (A*A/2 + (5 - T + 9*C + 4*C*C) * pow(A,4)/24
         + (61 - 58*T + T*T + 600*C - 330*ep2) * pow(A,6)/720));

    // Ajusta para hemisfério sul
    if (lat_deg < 0)
        *y += 10000000.0;
}


void reduzirEscala(Node pontos[], int n, int redutor)
{
	double minX = pontos[0].x, minY = pontos[0].y;
    for (int i = 1; i < n; i++) {
        if (pontos[i].x < minX) minX = pontos[i].x;
        if (pontos[i].y < minY) minY = pontos[i].y;
    }
    for (int i = 0; i < n; i++) {
        pontos[i].x = (pontos[i].x - minX) / redutor; // reduzir escala
        pontos[i].y = (pontos[i].y - minY) / redutor;
    } 
}



void parse_osm(const char* filename) 
{
	char arqSaida[100];
	int posPonto = RAt(".", filename);

	strcpy(arqSaida, Left(filename, posPonto));
	strcat(arqSaida, ".poly");
	
    FILE* f = fopen(filename, "r");
    if (!f) {
        perror("Erro ao abrir o arquivo");
        return;
    }
    FILE* outFile = fopen(arqSaida, "w");

    char line[1024];
    int inside_way = 0;
    Way current_way;
    current_way.count = 0;

    while (fgets(line, sizeof(line), f)) {
        // Verifica se é um nó
        if (strstr(line, "<node") && strstr(line, "lat=") && strstr(line, "lon=")) {
            char id_str[64], lat_str[64], lon_str[64];
            extract_attr(line, "id=", id_str);
            extract_attr(line, "lat=", lat_str);
            extract_attr(line, "lon=", lon_str);

            if (total_nodes < MAX_NODES) {
                nodes[total_nodes].id_original = atoll(id_str);
                nodes[total_nodes].lat = atof(lat_str);
                nodes[total_nodes].lon = atof(lon_str);
                
                converter_para_utm(nodes[total_nodes].lat, nodes[total_nodes].lon, &nodes[total_nodes].x, &nodes[total_nodes].y);
                
                nodes[total_nodes].id_interno = total_nodes;
                total_nodes++;
            }
        }

        // Verifica se é o início de uma via
        else if (strstr(line, "<way")) {
            inside_way = 1;
            current_way.count = 0;
        }

        // Verifica se é um nó dentro de uma via
        else if (inside_way && strstr(line, "<nd")) {
            char ref_str[64];
            extract_attr(line, "ref=", ref_str);
            long long ref_id = atoll(ref_str);
            int index = get_node_index(ref_id);
            if (index != -1 && current_way.count < MAX_WAY_NODES) {
                current_way.node_ids[current_way.count++] = index;
            }
        }

        // Fim de uma via
        else if (inside_way && strstr(line, "</way>")) {
            inside_way = 0;
            if (current_way.count > 1 && total_ways < MAX_WAYS) {
                ways[total_ways++] = current_way;
            }
        }
    }

    fclose(f);

	 //reduzirEscala(nodes, total_nodes, 5);
	 reduzirEscala(nodes, total_nodes, 2);
	 
	/* Obter máximo valor da coordenada Y e fazer rotação vertical
	 tal que a coordenada (0,0) seja no lado superior esquerdo,
	 característica do componente TImage.
	*/
	double maxY = nodes[0].y;
	for (int i = 1; i < total_nodes; i++) {
		if (nodes[i].y > maxY)
			maxY = nodes[i].y;
	}	 
	
	for (int i = 0; i < total_nodes; i++) {
		nodes[i].y = maxY - nodes[i].y;	
	}

	 	  
    // Imprime nós
    //printf("Vértices (ID interno, latitude, longitude):\n");
    fprintf(outFile, "%d\t%d\t%d\t%d\n", total_nodes, 2, 0, 1);
    for (int i = 0; i < total_nodes; i++) {
        fprintf(outFile, "%d\t%f\t%f\n", nodes[i].id_interno, nodes[i].x, nodes[i].y);
        
    }

    // Imprime arestas
    //printf("\nArestas (pares de vértices consecutivos por via):\n");
    int numID = 0;
    for (int i = 0; i < total_ways; i++) {
        for (int j = 0; j < ways[i].count - 1; j++) {
				numID++;
		  }
	}
    //printf("%d\t%d\n", numID, 1);
    fprintf(outFile, "%d\t%d\n", numID, 1);
    
    numID = 0;
    for (int i = 0; i < total_ways; i++) {
        for (int j = 0; j < ways[i].count - 1; j++) {
            int from = ways[i].node_ids[j];
            int to = ways[i].node_ids[j + 1];
            //printf("%d\t%d\t%d\t%d\n", numID++, from, to, 0);
            fprintf(outFile, "%d\t%d\t%d\t%d\n", numID++, from, to, 0);
            
        }
    }
    //printf("%d\n", 0);
    fprintf(outFile, "%d\n", 0);
    
    printf("Arquivo \"%s\" criado com sucesso.", arqSaida);
    fclose(outFile); 
}

  
int main(int argc, char* argv[]) 
{
    if (argc != 2) {
        fprintf(stderr, "Uso: %s arquivo.osm\n", argv[0]);
        return 1;
    }
    parse_osm(argv[1]);
    
    return 0;
}

