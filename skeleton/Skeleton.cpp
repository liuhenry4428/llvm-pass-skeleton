#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include <vector>
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    virtual bool runOnFunction(Function &F) {
      errs() << F.getName() << '\n';
      //Finds pointer operand
      Value * pointerOperand;
      std::vector<int64_t> constOffsets;
      for(auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
          arg->setName("funcArg");
          //errs() << arg->getName() << "\n";
          assert(arg->getNumUses() == 1);
          for(auto argUse = arg->use_begin(); argUse != arg->use_end(); ++argUse){
            auto argUseValue = argUse->getUser();
            if(auto * inst  = dyn_cast<StoreInst>(argUseValue)){
              pointerOperand = inst->getPointerOperand();
              pointerOperand->setName("pointerOperand");
            }
          }
        }

      //Finds rec argument and checks if it's constant negative offset from funcArg
      for (auto &bb : F) {
        for (auto &instruction : bb) {
          if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
            if (Function *calledFunction = callInst->getCalledFunction()) {
              if (calledFunction->getName().startswith(F.getName())) {
                for(auto recArg = callInst->arg_begin(); recArg != callInst->arg_end(); ++recArg) {
                  auto* recArgValue =recArg->get();
                  //TODO: assert recArgValue is a subtract instructon
                  for(auto recArgUse = recArgValue->use_begin(); recArgUse != recArgValue->use_end(); ++recArgUse){
                    auto * recArgUseValue = recArgUse->get();
                    auto * recArgUseValueInst = dyn_cast<Instruction>(recArgUseValue) ;
                    auto * recArgUseValueInstOperand = recArgUseValueInst->getOperand(0);
                    recArgUseValueInstOperand ->setName("loadedOriginalArg");
                    recArgUseValue->setName("recArg");

                    if(auto * loadInstCast = dyn_cast<LoadInst>(recArgUseValueInstOperand)) {
                      auto  loadInstCastOperandName = loadInstCast->getPointerOperand()->getName();
                      errs() << "Load instruction source "<<loadInstCastOperandName<< '\n';
                      assert(loadInstCastOperandName.startswith("pointerOperand"));
                      auto * constOperand = recArgUseValueInst->getOperand(1);
                      if(auto * constOperandCasted = dyn_cast<ConstantInt>(constOperand)){
                        auto intValue = constOperandCasted->getSExtValue();
                        errs() << "Const Offset Amount: " << intValue << '\n';
                        assert(intValue>0);
                        constOffsets.push_back(intValue);
                      }
                      else assert(0);
                    }
                    else assert(0);
                    
                  }
                }
              }
            }
          }
        }
      }
      errs() << "I saw a function called " << F.getName() << "!\n";
      return false;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
