#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/CFG.h"
#include "llvm/Analysis/CFG.h"
#include <vector>
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}

    // Assume all comparisons for the function argument to base case is done only with equality comparison
    ConstantInt * findBaseCaseVal(BasicBlock &bb, BasicBlock &prevbb){
      auto it = bb.end(); //Iterator
        //only need 1 iteration
      if(BranchInst * branch = dyn_cast<BranchInst>(&*it)){ //Suspicious &*it
        if(branch->isUnconditional()){
          for(auto pred : predecessors(&bb)){
            ConstantInt * dfsResult = findBaseCaseVal(*pred, bb);
            if(dfsResult != nullptr) return dfsResult;
          }
          return nullptr;
        }
        else if(branch->isConditional()){
          //Return null if previous BB is not the true destination
          auto trueDest = branch->op_end()->get(); //op_end is the true destination
          if (auto * castedTrueDest = dyn_cast<BasicBlock>(trueDest)){
            if(!(castedTrueDest == &prevbb)) return nullptr;
          }
          if(CmpInst * conditionInst = dyn_cast<CmpInst>(branch->getCondition())){
            if(conditionInst->isEquality()){
              assert(conditionInst->getNumOperands() == 2); //you never know...
              auto * op1 = conditionInst->getOperand(0);
              auto * op2 = conditionInst->getOperand(1);
              //Check that one of the arguments is the function argument, then return the constant int it's compared to
              //Throw if we compare with a non-ConstantInt
              if(op1->hasName() && op1->getName().startswith("funcArg")){
                if(auto ret = dyn_cast<ConstantInt>(op2)) return ret; else assert(0); 
              }
              if(op2->hasName() && op2->getName().startswith("funcArg")){
                if(auto ret = dyn_cast<ConstantInt>(op1)) return ret; else assert(0);
              }
            }
          }
          //Loop over block preds
          for(auto pred : predecessors(&bb)){
            ConstantInt * dfsResult = findBaseCaseVal(*pred, bb);
            if(dfsResult != nullptr) return dfsResult;
          }
          return nullptr;
        }
      }
    }

    virtual bool runOnFunction(Function &F) {
      if(F.getName().startswith("main")) return false;
      // errs() << F.getName() << '\n';
      //Finds pointer operand
      Value * pointerOperand;
      std::vector<int64_t> constOffsets;
      // for(auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
      //     arg->setName("funcArg");
      //     //errs() << arg->getName() << "\n";
      //     assert(arg->getNumUses() == 1);
      //     for(auto argUse = arg->use_begin(); argUse != arg->use_end(); ++argUse){
      //       auto argUseValue = argUse->getUser();
      //       if(auto * inst  = dyn_cast<StoreInst>(argUseValue)){
      //         pointerOperand = inst->getPointerOperand();
      //         pointerOperand->setName("pointerOperand");
      //       }
      //     }
      //   }

      // //Finds rec argument and checks if it's constant negative offset from funcArg
      // for (auto &bb : F) {
      //   for (auto &instruction : bb) {
      //     if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
      //       if (Function *calledFunction = callInst->getCalledFunction()) {
      //         if (calledFunction->getName().startswith(F.getName())) {
      //           for(auto recArg = callInst->arg_begin(); recArg != callInst->arg_end(); ++recArg) {
      //             auto* recArgValue =recArg->get();
      //             //TODO: assert recArgValue is a subtract instructon
      //             for(auto recArgUse = recArgValue->use_begin(); recArgUse != recArgValue->use_end(); ++recArgUse){
      //               auto * recArgUseValue = recArgUse->get();
      //               auto * recArgUseValueInst = dyn_cast<Instruction>(recArgUseValue) ;
      //               auto * recArgUseValueInstOperand = recArgUseValueInst->getOperand(0);
      //               recArgUseValueInstOperand ->setName("loadedOriginalArg");
      //               recArgUseValue->setName("recArg");

      //               if(auto * loadInstCast = dyn_cast<LoadInst>(recArgUseValueInstOperand)) {
      //                 auto  loadInstCastOperandName = loadInstCast->getPointerOperand()->getName();
      //                 //errs() << "Load instruction source "<<loadInstCastOperandName<< '\n';
      //                 assert(loadInstCastOperandName.startswith("pointerOperand"));
      //                 auto * constOperand = recArgUseValueInst->getOperand(1);
      //                 if(auto * constOperandCasted = dyn_cast<ConstantInt>(constOperand)){
      //                   auto intValue = constOperandCasted->getSExtValue();
      //                   errs() << "Const Offset Amount: " << intValue << '\n';
      //                   assert(intValue>0);
      //                   constOffsets.push_back(intValue);
      //                 }
      //                 else assert(0);
      //               }
      //               else assert(0);
                    
      //             }
      //           }
      //         }
      //       }
      //     }
      //   }
      // }

      //Find Return Stmt
      ReturnInst * returnStmt = NULL;
      for(auto &bb : F) for (auto &instruction : bb) 
        if(auto * retInst = dyn_cast<ReturnInst>(&instruction)) 
        {
          errs()<<"FOUND RET!\n";
          if (returnStmt == NULL) returnStmt = retInst; else assert(false);
        }
      assert(returnStmt != NULL);
      Value * returnValue = returnStmt->getReturnValue(); assert(returnValue != NULL);
      std::vector<int> baseCaseArg;
      std::vector<int> baseCaseVal;
      returnValue->setName("returnValue");
      if(auto * returnPhi = dyn_cast<PHINode>(returnValue)){
        for(int i =0; i<returnPhi->getNumIncomingValues(); i++){
          errs()<<"@@@@@@@@@@@@@";
          auto * inBlock = returnPhi->getIncomingBlock(i);
          auto * inValue = returnPhi->getIncomingValue(i);
          if(!isa<ConstantInt>(inValue)) continue;

        }
      }
      else{
        assert(false);
      }
      return false;
    }
  };
}

char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
