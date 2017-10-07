/*  Programa de demonstração de uso de sockets UDP em C no Linux
 *  Funcionamento:
 * T1
 *  O programa cliente envia uma msg para o servidor. Essa msg é uma palavra.
 *  O servidor envia outra palavra como resposta.
 *  Estado Atual: Com controle de nível, com funcionamento periódico em CLOCK_MONOTONIC;
 *                com socket global, sem getvalor e mandaValor, sem leitura de intervalo de tempo;
 * 				  com gravação em arquivo;
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>

#define FALHA 1

#define	TAM_MEU_BUFFER	1000

#define NSEC_PER_SEC    (1000000000) /* The number of nsecs per sec. */

#define B 4
#define P 1000
#define S 4184
#define R 000.1
#define KpH 0.5
#define offset 0.8

/*configs e funcoes de rede*/

int cria_socket_local(void)
{
	int socket_local;		/* Socket usado na comunicacão */

	socket_local = socket( PF_INET, SOCK_DGRAM, 0);
	if (socket_local < 0) {
		perror("socket");
		return 0;
	}
	return socket_local;
}


struct sockaddr_in cria_endereco_destino(char *destino, int porta_destino)
{
	struct sockaddr_in servidor; 	/* Endereço do servidor incluindo ip e porta */
	struct hostent *dest_internet;	/* Endereço destino em formato próprio */
	struct in_addr dest_ip;		/* Endereço destino em formato ip numérico */

	if (inet_aton ( destino, &dest_ip ))
		dest_internet = gethostbyaddr((char *)&dest_ip, sizeof(dest_ip), AF_INET);
	else
		dest_internet = gethostbyname(destino);

	if (dest_internet == NULL) {
		fprintf(stderr,"Endereço de rede inválido\n");
		exit(FALHA);
	}

	memset((char *) &servidor, 0, sizeof(servidor));
	memcpy(&servidor.sin_addr, dest_internet->h_addr_list[0], sizeof(servidor.sin_addr));
	servidor.sin_family = AF_INET;
	servidor.sin_port = htons(porta_destino);

	return servidor;
}


void envia_mensagem(int socket_local, struct sockaddr_in endereco_destino, char *mensagem)
{
	/* Envia msg ao servidor */

	if (sendto(socket_local, mensagem, strlen(mensagem)+1, 0, (struct sockaddr *) &endereco_destino, sizeof(endereco_destino)) < 0 )
	{
		perror("sendto");
		return;
	}
}


int recebe_mensagem(int socket_local, char *buffer, int TAM_BUFFER)
{
	int bytes_recebidos;		/* Número de bytes recebidos */

	/* Espera pela msg de resposta do servidor */
	bytes_recebidos = recvfrom(socket_local, buffer, TAM_BUFFER, 0, NULL, 0);
	if (bytes_recebidos < 0)
	{
		perror("recvfrom");
	}

	return bytes_recebidos;
}


/*FUNÇÃO GETVALOR*/
float getValor(char msg[], int socket_local, struct sockaddr_in endereco_destino){

	int i, porta_destino = 12345;

	char msg_recebida[1000];

	envia_mensagem(socket_local, endereco_destino, msg);
	int nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
	for(i = 0; i <= 4; i++){
		msg_recebida[i] = msg_recebida[i+3];
	}
	msg_recebida[5] = '\0';
	//printf(msg_recebida);
	//printf("\n");
	return atof(msg_recebida);

}


/* MANDA VALOR, QUEBRADASSO, OCORRENCIA DE SEGFAULT
 * void mandaValor(char tipo[], float valor){

	int porta_destino = 12345;
	struct sockaddr_in endereco_destino = cria_endereco_destino("localhost", 12345);
	int socket_local = cria_socket_local();
	char strValor[20], strMsg[20];

	printf("%f\n",valor);
	sprintf(*strValor, "%f",valor);
	strValor[5] = '\0';
	printf(strValor);
	strcat(tipo, strValor);
	strcpy(strMsg, tipo);
	printf(strMsg);
	printf("  <-Mensagem enviada\n");
	envia_mensagem(socket_local, endereco_destino, strMsg);

}
*/


//--------------------------------------------------------------------------
//--------------------------------------MAIN--------------------------------
//--------------------------------------------------------------------------

int main(int argc, char *argv[])
{
	if (argc < 4) {
		fprintf(stderr,"Uso: udpcliente endereço porta palavra \n");
		fprintf(stderr,"onde o endereço é o endereço do servidor \n");
		fprintf(stderr,"porta é o número da porta do servidor \n");
		fprintf(stderr,"palavra é a palavra que será enviada ao servidor \n");
		fprintf(stderr,"exemplo de uso:\n");
		fprintf(stderr,"   udpcliente baker.das.ufsc.br 1234 \"ola\"\n");
		exit(FALHA);
	}

	///* segmento periódico */
	 	//struct timespec t, tfin;
	 	//int interval = 500000000; /* (1/2)s */
	 	////500000000 =  (1/2)s

	//clock_gettime(CLOCK_MONOTONIC ,&t);
		///* start after one second */
		//t.tv_sec++;

	/*config de rede*/
	int porta_destino = atoi( argv[2]);
	//pega porta, converte pra inteiro, salva na variavel

	int socket_local = cria_socket_local();

	struct sockaddr_in endereco_destino = cria_endereco_destino(argv[1], porta_destino);

	FILE *dados;
	int temporizador, i = 0, iteract = 0;
	char msg_recebida[1000], strValor[20], strMsg[20];
	int nrec;
	float H, Ni, No, Uh;

	float Href = atof (argv[3]);
	printf("	Referencia de altura = %.1f\n",Href);

	/*EXEMPLO DE COMUNICACAO
		envia_mensagem(socket_local, endereco_destino, argv[3]);
	nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
	msg_recebida[ nrec ] = '\0';
	printf("Mensagem de resposta com %d bytes >>>%s\n", nrec, msg_recebida);
	*
	* 	H = getValor("sh-0");
		printf("	Altura do reservatorio em %.1f\n",H);
		No = getValor("sno0");
		printf("	Fluxo de saída de agua em %.1f\n",H);
		T = getValor("st-0");
		printf("	Temperatura ambiente em %.1f\n\n",H);
	*/

	// MALHA DE CONTROLE de ALTURA

	No = getValor("sno0", socket_local, endereco_destino);
		/*envia_mensagem(socket_local, endereco_destino, "sno0");
		nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
		for(i = 0; i <= 4; i++){
			msg_recebida[i] = msg_recebida[i+3];
		}
		msg_recebida[5] = '\0';
		//printf(msg_recebida);
		//printf("\n");
		No = atof(msg_recebida);*/

	printf("	Fluxo de saída de agua em %.1f\n",No);
	//valor inicial de Ni = No;
	Ni = No;
	float ke = (B * P) / No;
	dados = fopen("arquivodados.txt", "w");
	fprintf(dados, " 200 Medidas de altura H\n");


	while(1){
		//clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);

		iteract++;

		/* do the stuff */

		H = getValor("sh-0", socket_local, endereco_destino);
			/*envia_mensagem(socket_local, endereco_destino, "sh-0");
			nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
			for(i = 0; i <= 4; i++){
				msg_recebida[i] = msg_recebida[i+3];
			}
			msg_recebida[5] = '\0';
			//printf(msg_recebida);
			//printf("\n");
			H = atof(msg_recebida);*/

			/*ESCRITA DE DADOS EM ARQUIVO
			fprintf(dados, " %f\n",H);
			if (iteract == 100000){ fclose(dados); }
			*/


		Uh = ke * KpH * (Href - H);
		Ni = Ni*offset + Uh;
		if (Ni < 0){ Ni = 0;}
		if (Ni > 100){ Ni = 100;}
		//Ni = Ni*offset + Uh;

		//mandaValor
		sprintf(strValor, "%f",Ni);
		strValor[5] = '\0';
		strcpy(strMsg, "ani");
		strcat(strMsg, strValor);
		envia_mensagem(socket_local, endereco_destino, strMsg);
		nrec = recebe_mensagem(socket_local, msg_recebida, 1000);
		msg_recebida[ nrec ] = '\0';

		/*Aquisição de tempo
		//clock_gettime(CLOCK_MONOTONIC ,&tfin);
		//float varT = tfin.tv_nsec - t.tv_nsec;
		//printf(" Tempo = %.0f\n",varT); */

	 /*printer*/
	 if (iteract%100 == 0){
			 printf("Iteração: %d\n",iteract);
		   printf("Controle = %f \n",Uh);
			 printf("%s  <-Mensagem enviada\n", strMsg);
			 printf("Mensagem de resposta com %d bytes >>>%s\n", nrec, msg_recebida);
	 }
		/*temporizador simples*/
		temporizador = 0;
		while(temporizador<5000000){ temporizador++; }



		///* calculate next shot */
		//clock_gettime(CLOCK_MONOTONIC ,&t);
        //t.tv_nsec += interval;
        //while (t.tv_nsec >= NSEC_PER_SEC) {
                //t.tv_nsec -= NSEC_PER_SEC;
                //t.tv_sec++;
        //}

	}
	fclose(dados);

}
