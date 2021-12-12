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
uint8_t* counters;
//------------------------------------//
//        Predictor Functions         //
//------------------------------------//

// Initialize the predictor
//
void
init_predictor()
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

// Make a prediction for conditional branch instruction at PC 'pc'
// Returning TAKEN indicates a prediction of taken; returning NOTTAKEN
// indicates a prediction of not taken
//
uint8_t
make_prediction(uint32_t pc)
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
      uint32_t pred = counters[counter_idx % bufferSize];
      if (pred < 2) 
        return NOTTAKEN;
      else
        return TAKEN;
    case TOURNAMENT:
    case CUSTOM:
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
