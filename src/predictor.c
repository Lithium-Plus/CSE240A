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
const char *studentName = "NAME";
const char *studentID   = "PID";
const char *email       = "EMAIL";

//------------------------------------//
//      Predictor Configuration       //
//------------------------------------//

// Handy Global for use in output routines
const char *bpName[4] = { "Static", "Gshare",
                          "Tournament", "Custom" };

int ghistoryBits; // Number of bits used for Global History
int lhistoryBits; // Number of bits used for Local History
int pcIndexBits;  // Number of bits used for PC index
int bpType;       // Branch Prediction Type
int verbose;

//------------------------------------//
//      Predictor Data Structures     //
//------------------------------------//

//
//TODO: Add your own Branch Predictor data structures here
//

uint32_t branchHistoryRegister;
uint16_t bufferSize;
uint8_t histBits = 13;
uint32_t counter_idx;
// uint8_t* counters;
uint8_t counters[8192];

uint8_t* localPHT_tourament;
uint8_t* globalPHT_tourament;
uint32_t* indexTable_tourament;
uint8_t* chooserTable_tourament;
int ghr_tourament;
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void init_predictor()
{
  //
  //TODO: Initialize Branch Predictor Data Structures
  //
  bufferSize = (int) pow(2, histBits);
  uint8_t counters[bufferSize];
  branchHistoryRegister = branchHistoryRegister & (0 << histBits);
  printf("pred here");
  
  // strong not taken 0, weak not taken 1, weak taken 2, strong taken 3
  for (int i = 0; i < bufferSize; i++){
    counters[i] = 1;
  }

}

void init_tourament() {
    ghr_tourament = 0;
    int localSize = getSize(lhistoryBits);
    int globalSize = getSize(ghistoryBits);
    int indexTableSize = getSize(pcIndexBits);

    localPHT_tourament = (uint8_t*)malloc (localSize * sizeof(uint8_t));
    globalPHT_tourament = (uint8_t*)malloc (globalSize * sizeof(uint8_t));
    indexTable_tourament = (uint32_t*)malloc (indexTableSize * sizeof(uint32_t));
    chooserTable_tourament = (uint8_t*)malloc (globalSize * sizeof(uint8_t));

    for (int i = 0; i < localSize; i++) {
        localPHT_tourament[i] = WN;
    }

    for (int i = 0; i < globalSize; i++) {
        globalPHT_tourament[i] = WN;
    }

    for (int i = 0; i < indexTableSize; i++) {
        indexTable_tourament[i] = 0;
    }

    for (int i = 0; i < globalSize; i++) {
        chooserTable_tourament[i] = WG;
    }
}

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t make_prediction(uint32_t pc)
{
  //
  //TODO: Implement prediction scheme
  //

  // Make a prediction based on the bpType
  switch (bpType) {
    case STATIC:
      return TAKEN;
    case GSHARE:
      
      counter_idx = pc ^ branchHistoryRegister;
      uint8_t pred = counters[counter_idx % bufferSize];
      if (pred < 2) 
        return NOTTAKEN;
      else
        return TAKEN;
    case TOURNAMENT:
        return predict_tourament(pc);
    case CUSTOM:
    default:
      break;
  }

  // If there is not a compatable bpType then return NOTTAKEN
  return NOTTAKEN;
}
uint8_t predict_tourament(uint32_t pc) {
    uint32_t pcMask = getMask(pcIndexBits);

    int pcIndex = pc & pcMask;
    int localPHTIndex = indexTable_tourament[pcIndex];
    int localPredict = localPHT_tourament[localPHTIndex];
    int globalPredict = globalPHT_tourament[ghr_tourament];
    int chooser = chooserTable_tourament[ghr_tourament];

    if (chooser <= WG) {
        return (globalPredict <= WN) ? NOTTAKEN : TAKEN;
    } else {
        return (localPredict <= WN) ? NOTTAKEN : TAKEN;
    }
}

void train_tourament(uint32_t pc, uint8_t outcome) {
    //Train Local Predictor
    // 1. Train localPHT Table
    uint32_t pcMask = getMask(pcIndexBits);
    uint32_t localHistoryMask = getMask(lhistoryBits);

    int pcIndex = pc & pcMask;
    int localPHTIndex = indexTable_tourament[pcIndex];
    int localPrediction = localPHT_tourament[localPHTIndex];
    int localResult = (localPrediction <= WN) ? NOTTAKEN : TAKEN;

    localPHT_tourament[localPHTIndex] = updateTwoBit(localPHT_tourament[localPHTIndex], outcome);

    // 2. Train Index Table
    indexTable_tourament[pcIndex] = ((indexTable_tourament[pcIndex] << 1) | outcome) & localHistoryMask;


    //Train Global Predictor
    uint32_t globalHistoryMask = getMask(ghistoryBits);
    int globalPHTIndex = ghr_tourament & globalHistoryMask;
    int globalPrediction = globalPHT_tourament[globalPHTIndex];
    int globalResult = (globalPrediction <= WN) ? NOTTAKEN: TAKEN;

    globalPHT_tourament[globalPHTIndex] = updateTwoBit(globalPHT_tourament[globalPHTIndex], outcome);

    ghr_tourament = ((ghr_tourament << 1) | outcome) & globalHistoryMask;

    //Train Chooser
    if (globalResult != localResult) {
        uint8_t choice = chooserTable_tourament[globalPHTIndex];
        if (globalResult == outcome) {
            if (choice != SG) {
                chooserTable_tourament[globalPHTIndex] = choice - 1;
            }
        } else {
            if (choice != SL) {
                chooserTable_tourament[globalPHTIndex] = choice + 1;
            }
        }
    }

}

// Train the predictor the last executed branch at PC 'pc' and with
// outcome 'outcome' (true indicates that the branch was taken, false
// indicates that the branch was not taken)
//
void
train_predictor(uint32_t pc, uint8_t outcome)
{
  //
  //TODO: Implement Predictor training
  //
  uint32_t counter_idx = pc ^ branchHistoryRegister;
  uint32_t pred = counters[counter_idx % bufferSize];
  if (outcome == TAKEN){
    if (pred < 3){
      pred += 1;
    }
  }
  else{
    if (pred > 0){
      pred -= 1;
    }
  }
  counters[counter_idx % bufferSize] = pred;
  branchHistoryRegister = branchHistoryRegister << 1;
  branchHistoryRegister = branchHistoryRegister | outcome;
}


uint32_t getMask(uint32_t bit) {
    return (1 << bit) - 1;
}

uint32_t getSize(uint32_t bit) {
    return (1 << bit);
}

uint8_t updateTwoBit(u_int8_t currentState, u_int8_t outcome) {
    if (outcome == TAKEN) {
        if (currentState == ST) {
            return currentState;
        } else {
            return currentState + 1;
        }
    } else {
        if (currentState == SN) {
            return currentState;
        } else {
            return currentState - 1;
        }
    }
}