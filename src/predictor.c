//========================================================//
//  predictor.c                                           //
//  Source file for the Branch Predictor                  //
//                                                        //
//  Implement the various branch predictors below as      //
//  described in the README                               //
//========================================================//
#include <stdio.h>

#include <math.h>
#include "predictor.h"

//
// TODO:Student Information
//



const char *studentName = "Amardeep Ramnani";
const char *studentID   = "A59005452";
const char *email       = "aramnani@ucsd.edu";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

//Metapredictor States
#define chooser_local_yes 0
#define chooser_local_probably 1
#define chooser_gshare_probably 2
#define chooser_gshare_yes 3

//Perceptron
#define number_of_weights 25
#define number_of_bits_for_weight 8
#define weight_max_value (pow(2, number_of_bits_for_weight - 1) - 1)
#define weight_min_value ((weight_max_value + 1) * (-1))
#define perceptron_table_size (32 * 1024)
#define number_of_perceptrons ((int)(perceptron_table_size / ((number_of_weights + 1) * number_of_bits_for_weight)))
#define threshold (1.93 * number_of_weights + 14)

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

//define number of bits required for indexing the BHT here. 
int ghistoryBits = 14; // Number of bits used for Global History
int bpType;       // Branch Prediction Type
int verbose;


//tournament
int tournament_local_predictor_pattern_length = 11;
int tournament_gshare_history_pattern_length  = 11;
int pc_bits_used_tournament = 11;
//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//
//Perceptron
//int global_history_bits_perceptron = 28;
//int pc_bits_for_perceptron = 10;
//int number_of_bits_for_weight = 8;



//TODO: Add your own Branch Predictor data structures here
//
//gshare
uint8_t *bht_gshare;
uint64_t ghistory;

//tournament

uint8_t *tournament_metapredictor;
uint8_t *tournament_gshare;
uint16_t *tournament_local_predictor;
uint8_t *tournament_local_predictor_BHT;
uint64_t tournament_ghistory;


//perceptron

int perceptron_table[number_of_perceptrons][number_of_weights + 1];
//int pc_lower_bits;
int perceptron_ghistory;
int threshold_check;
int y;
//int perceptron_ghistory;
uint8_t prediction;

//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//

//gshare functions
void init_gshare() {
 int bht_entries = 1 << ghistoryBits;
  bht_gshare = (uint8_t*)malloc(bht_entries * sizeof(uint8_t));
  int i = 0;
  for(i = 0; i< bht_entries; i++){
    bht_gshare[i] = WN;
  }
  ghistory = 0;
}

//tournament functions

void init_tournament() {
uint32_t local_predictor_entries = 1 << pc_bits_used_tournament;

tournament_local_predictor =  (uint16_t*)malloc(local_predictor_entries * sizeof(uint16_t));

int i =0;

for (i =0; i < local_predictor_entries; i++) {
tournament_local_predictor[i] = 0;
}

uint32_t local_predictor_BHT_entries = 1 << tournament_local_predictor_pattern_length; 
tournament_local_predictor_BHT =  (uint8_t*)malloc(local_predictor_BHT_entries * sizeof(uint8_t));

for (i =0; i < local_predictor_BHT_entries; i++) {
tournament_local_predictor_BHT[i] = WN;
}

uint32_t bht_entries_gshare = 1 << tournament_gshare_history_pattern_length;
tournament_gshare = (uint8_t*)malloc(bht_entries_gshare * sizeof(uint8_t));

for(i = 0; i< bht_entries_gshare; i++){
  tournament_gshare[i] = WN;
}

uint32_t meta_predictor_entries = 1 << pc_bits_used_tournament;  
tournament_metapredictor = (uint8_t*)malloc(meta_predictor_entries * sizeof(uint8_t));

for(i = 0; i< meta_predictor_entries; i++){
  tournament_metapredictor[i] = chooser_local_probably;
}

tournament_ghistory = 0;

}

//perceptron

void init_perceptron() {


int i , j;

threshold_check = 0;

for (i = 0; i < number_of_perceptrons; i++) {

   for (j = 0; j <= number_of_weights + 1; j++) {
        perceptron_table[i][j] = 0;
}  

}

perceptron_ghistory = 0;

}

uint8_t 
gshare_predict(uint32_t pc) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;
  switch(bht_gshare[index]){
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
      return NOTTAKEN;
  }
}

uint8_t 
tournament_predict(uint32_t pc) {

uint32_t meta_predictor_entries = 1 << pc_bits_used_tournament;
uint32_t pc_lower_bits = pc & (meta_predictor_entries - 1);

uint32_t local_predictor_BHT_index = tournament_local_predictor[pc_lower_bits] & (meta_predictor_entries - 1); 

uint32_t bht_entries_gshare = 1 << tournament_gshare_history_pattern_length;
uint32_t ghistory_lower_bits = tournament_ghistory & (bht_entries_gshare -1);
//uint32_t index_gshare_BHT = pc_lower_bits ^ ghistory_lower_bits;
uint32_t index_gshare_BHT = ghistory_lower_bits;


if (tournament_metapredictor[pc_lower_bits] == chooser_local_probably || tournament_metapredictor[pc_lower_bits] == chooser_local_yes) {

switch(tournament_local_predictor_BHT[local_predictor_BHT_index]) {
    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in LOCAL PREDICTOR BHT! in Predict\n");
      return NOTTAKEN;
  }

} 

else {

  switch(tournament_gshare[index_gshare_BHT]){

    case WN:
      return NOTTAKEN;
    case SN:
      return NOTTAKEN;
    case WT:
      return TAKEN;
    case ST:
      return TAKEN;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT! in Predict\n");
      return NOTTAKEN;
  }
}

}


uint8_t 
perceptron_predict(uint32_t pc) {

prediction = 0;

uint32_t pc_lower_bits = pc % number_of_perceptrons;

y = perceptron_table[pc_lower_bits][0];

int key = 1;

uint8_t i;

for (i = 1; i < number_of_weights + 1; i++) {

if ((perceptron_ghistory&key) == 0) {
y -= perceptron_table[pc_lower_bits][i];
} 

else {
y += perceptron_table[pc_lower_bits][i];
}

key = key << 1;

}

threshold_check = y;

if (y >= 0) {
prediction = 1;
return TAKEN;
}
else {
prediction = 0;
return NOTTAKEN;
}


}

void
train_gshare(uint32_t pc, uint8_t outcome) {
  //get lower ghistoryBits of pc
  uint32_t bht_entries = 1 << ghistoryBits;
  uint32_t pc_lower_bits = pc & (bht_entries-1);
  uint32_t ghistory_lower_bits = ghistory & (bht_entries -1);
  uint32_t index = pc_lower_bits ^ ghistory_lower_bits;

  //Update state of entry in bht based on outcome
  switch(bht_gshare[index]){
    case WN:
      bht_gshare[index] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      bht_gshare[index] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      bht_gshare[index] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      bht_gshare[index] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT!\n");
  }

  //Update history register
  ghistory = ((ghistory << 1) | outcome); 
}

void
train_tournament(uint32_t pc, uint8_t outcome) {
//local predictor
uint32_t local_predictor_entries = 1 << pc_bits_used_tournament;
uint32_t pc_lower_bits = pc & (local_predictor_entries - 1);
uint32_t index_local_predictor = pc_lower_bits;
uint32_t index_local_predictor_BHT = tournament_local_predictor[pc_lower_bits] & (local_predictor_entries - 1);


uint8_t outcome_local_predictor = tournament_local_predictor_BHT[index_local_predictor_BHT];
if (outcome_local_predictor == WN || outcome_local_predictor == SN) {
outcome_local_predictor = NOTTAKEN;
} 

else {
outcome_local_predictor = TAKEN;
}

  switch(tournament_local_predictor_BHT[index_local_predictor_BHT]){
    case WN:
     tournament_local_predictor_BHT[index_local_predictor_BHT] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
     tournament_local_predictor_BHT[index_local_predictor_BHT] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
     tournament_local_predictor_BHT[index_local_predictor_BHT] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
     tournament_local_predictor_BHT[index_local_predictor_BHT] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in LOCAL PREDICTOR BHT! in Training\n");
  }

  tournament_local_predictor[pc_lower_bits] = ((tournament_local_predictor[pc_lower_bits] << 1) | outcome); 
 // printf("pc_lower_bits : %d and the pattern is  %d \n", pc_lower_bits, tournament_local_predictor[pc_lower_bits]);

//gshare

  uint32_t bht_entries_gshare = 1 << tournament_gshare_history_pattern_length;
  uint32_t ghistory_lower_bits = tournament_ghistory & (bht_entries_gshare -1);
 // uint32_t index_gshare_BHT = pc_lower_bits ^ ghistory_lower_bits;
 uint32_t index_gshare_BHT = ghistory_lower_bits;


  uint8_t outcome_gshare = tournament_gshare[index_gshare_BHT];

if (outcome_gshare == WN || outcome_gshare == SN) {
outcome_gshare = NOTTAKEN;
} 

else {
outcome_gshare = TAKEN;
}




  //Update state of entry in bht based on outcome
  switch(tournament_gshare[index_gshare_BHT]){
    case WN:
     tournament_gshare[index_gshare_BHT] = (outcome==TAKEN)?WT:SN;
      break;
    case SN:
      tournament_gshare[index_gshare_BHT] = (outcome==TAKEN)?WN:SN;
      break;
    case WT:
      tournament_gshare[index_gshare_BHT] = (outcome==TAKEN)?ST:WN;
      break;
    case ST:
      tournament_gshare[index_gshare_BHT] = (outcome==TAKEN)?ST:WT;
      break;
    default:
      printf("Warning: Undefined state of entry in GSHARE BHT! in Training\n");
  }


  tournament_ghistory = ((tournament_ghistory << 1) | outcome); 

//MetaPredictor
switch(tournament_metapredictor[pc_lower_bits]) {

case chooser_local_yes:
tournament_metapredictor[pc_lower_bits] = (outcome == outcome_gshare && outcome != outcome_local_predictor) ? chooser_local_probably : chooser_local_yes;
break;

case chooser_local_probably:
tournament_metapredictor[pc_lower_bits] = (outcome == outcome_gshare && outcome != outcome_local_predictor) ? chooser_gshare_probably : chooser_local_yes;
break;

case chooser_gshare_probably:
tournament_metapredictor[pc_lower_bits] = (outcome == outcome_local_predictor) ? chooser_local_probably : chooser_gshare_yes;
break;

case chooser_gshare_yes:
tournament_metapredictor[pc_lower_bits] = (outcome == outcome_local_predictor) ? chooser_gshare_probably : chooser_gshare_yes;
break;

default:
printf("Warning: Undefined state of entry in METAPREDICTOR!\n");
}

}


void
train_perceptron(uint32_t pc, uint8_t outcome) {


uint32_t pc_lower_bits = pc % number_of_perceptrons;

uint8_t i;

int weight_change;

int key = 1;


if (prediction != outcome || (threshold_check < threshold && threshold_check > ((-1) * threshold))) {

if ((perceptron_table[pc_lower_bits][0] < weight_max_value) && (perceptron_table[pc_lower_bits][0] > weight_min_value)) {

if (outcome == TAKEN) {
perceptron_table[pc_lower_bits][0] = perceptron_table[pc_lower_bits][0] + 1;
}


else {
perceptron_table[pc_lower_bits][0] =  perceptron_table[pc_lower_bits][0] - 1;;
}

}

for (i = 1; i < number_of_weights + 1; i++) {

if (((outcome == TAKEN) && ((perceptron_ghistory & key) != 0)) || ((outcome == NOTTAKEN) && ((perceptron_ghistory & key) == 0))) {
  weight_change = 1;
}
else {
  weight_change = -1;
}

if ((perceptron_table[pc_lower_bits][i] < weight_max_value) && (perceptron_table[pc_lower_bits][i] > weight_min_value)) {
perceptron_table[pc_lower_bits][i] = perceptron_table[pc_lower_bits][i] + weight_change;
}

key = key << 1;
}

}

perceptron_ghistory = ((perceptron_ghistory << 1) | outcome) ; 

}


void
cleanup_gshare() {
  free(bht_gshare);
}



void
init_predictor()
{
  switch (bpType) {
    case STATIC:
    case GSHARE:
      init_gshare();
      break;
    case TOURNAMENT:
      init_tournament();
    break;
    case CUSTOM:
      init_perceptron();
     break;
    default:
      break;
  }
  
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
{

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      return gshare_predict(pc);
    case TOURNAMENT:
      return tournament_predict(pc);
    case CUSTOM:
    return perceptron_predict(pc);
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//

void
train_predictor(uint32_t pc, uint8_t outcome)
{

  switch (bpType) {
    case STATIC:
    case GSHARE:
      return train_gshare(pc, outcome);
    case TOURNAMENT:
      return train_tournament(pc, outcome);
    case CUSTOM:
      return train_perceptron(pc,outcome);
    default:
      break;
  }
  

}
