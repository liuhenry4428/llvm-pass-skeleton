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
#include <algorithm>
using namespace llvm;

namespace {
  struct SkeletonPass : public FunctionPass {
    static char ID;
    SkeletonPass() : FunctionPass(ID) {}


    /*deathBlitz is used to find base case arguments (i.e., arguments to the function that would not result in a recursive call)
      by doing a depth first search backwards basic block traversal from the phi node of a return to the first branch statement that is
      based off of a comparison between the argument and a constant. That constant is the base case argument for that particular phi node
      value. This function should be called per incoming basic block to the phi node where the incoming value is constant.
      This is a recursive dfs that returns nullptr if the comparison statement is not found, or that statement's ConstantInt if found.
      This is called deathBlitz because it has an extremely high chance of crashing. Correctness is produced through debugging of said crashes.
    */
    // Assume all comparisons for the function argument to base case is done only with equality comparison
    ConstantInt * deathBlitz(BasicBlock &bb, BasicBlock * prevbb){
      auto it = bb.rbegin(); //Iterator

      if(BranchInst * branch = dyn_cast<BranchInst>(&*it)){ //Suspicious &*it
        if(branch->isUnconditional()){
          //if unconditional, then just go to preds
        }
        //If conditional, check that the dest is previous dest, branch has comparison as conditional,
        //conditional is strictly equality, and that that conditional includes funcArg and a constInt
        else if(branch->isConditional()){
          //Return null if previous BB is not the true destination
          auto trueDest = branch->op_end()->get(); //op_end is the true destination
          if (auto * castedTrueDest = dyn_cast<BasicBlock>(trueDest)){
            if(prevbb != nullptr && !(castedTrueDest == &*prevbb)) return nullptr; //suspicious &*prevbb
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
        }
        // Recurse over all predecessors if not found.
        for(auto pred : predecessors(&bb)){
          void * returnedDeathBlitz = deathBlitz(*pred, &bb);
          if(returnedDeathBlitz != nullptr){
            if(ConstantInt * result = dyn_cast<ConstantInt>((Value *)returnedDeathBlitz)) return result; else assert(0);
          } 
        }
        return nullptr;
      }
      //If last inst not a branch
      else return nullptr;
    }



    virtual bool runOnFunction(Function &F) {
      if(F.getName().startswith("main") || F.arg_size()>1) return false;
      //TODO ensure function only has 1 arg??????????
      errs() <<"CURRENT FUNCTION: " <<F.getName() << '\n';
      //Finds pointer operand
      Value * pointerOperand;
      std::vector<int64_t> constOffsets;

      //Really only need 1 iteration
      for(auto arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
          arg->setName("funcArg");
      }

      //Finds rec argument and checks if it's constant negative offset from funcArg
      for (auto &bb : F) {
        std::vector<Value *> recCallReturns;
        std::vector<Value *> dependents;
        for (auto &instruction : bb) {
          //If inst is dependent on recursive call return
          if(Instruction * someInst = dyn_cast<Instruction>(&instruction)){
            bool exitFlag = false;//prevents double counting
            for(auto useIt = someInst->op_begin(); useIt != someInst->op_end(); ++useIt){
              auto * currentUse = useIt->get();
              //If dependent on rec call
              for(auto dep : dependents){
                if(dep == currentUse){
                  dependents.push_back(someInst);
                  exitFlag = true; 
                  break;
                }
              }
              if(exitFlag)break;
              //If dependent on something dependent on a rec call
              for(auto rcr : recCallReturns){
                if(rcr == currentUse){
                  dependents.push_back(someInst);
                  exitFlag = true;
                  break;
                }
              }
            }
            if(exitFlag) continue;
          }

          if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
            if (Function *calledFunction = callInst->getCalledFunction()) {
              if (calledFunction->getName().startswith(F.getName())) {
                callInst->setName("recCallReturn");
                recCallReturns.push_back(callInst);
                for(auto recArg = callInst->arg_begin(); recArg != callInst->arg_end(); ++recArg) { //should only need 1 iteration
                  auto* recArgValue =recArg->get();
                  recArgValue->setName("recArg");
                  if(auto * recArgInst = dyn_cast<Instruction>(recArgValue)){
                    if(auto * constArg = dyn_cast<ConstantInt>(recArgInst->getOperand(1))){
                      if(recArgInst->getOperand(0)->getName().startswith("funcArg")){
                        constOffsets.push_back(constArg->getSExtValue());
                        errs()<<"Found Offset: " <<constArg->getSExtValue() << '\n';
                      }
                    }
                    else return false;
                  }
                  else return false;

                  

                  //TODO: assert recArgValue is a subtract instructon <<<<<<VERY IMPORTANT
                }
              }
            }
          }
        }
        if(recCallReturns.size() != 0){
          errs()<< "RECURSIVE CALL RETURNS: \n";
          for(Value * i : recCallReturns){
            i->dump();
          }
        }
        if(dependents.size() != 0){
          errs()<< "DEPENDENTS: \n";
          for(Value * i : dependents){
            i->dump();
          }
        }
      }

      //Find (the 1) Return Stmt
      ReturnInst * returnStmt = NULL;
      for(auto &bb : F) for (auto &instruction : bb) 
        if(auto * retInst = dyn_cast<ReturnInst>(&instruction)) 
        {
          if (returnStmt == NULL) returnStmt = retInst; else assert(false);
        }
      assert(returnStmt != NULL);
      Value * returnValue = returnStmt->getReturnValue(); 
      assert(returnValue != nullptr);

      //Find base arguments
      std::vector<int> baseCaseArg;
      std::vector<int> baseCaseVal;
      returnValue->setName("returnValue");
      if(auto * returnPhi = dyn_cast<PHINode>(returnValue)){
        for(int i =0; i<returnPhi->getNumIncomingValues(); i++){
          auto * inBlock = returnPhi->getIncomingBlock(i);
          auto * inValue = returnPhi->getIncomingValue(i);

          //Found the recursive return value (ie non-base-case)
          if(!isa<ConstantInt>(inValue)) {
            continue;
          }
          errs()<<"BASE CASE ARGUMENT: ";
          inValue->dump();
          void * deathBlitzReturn = deathBlitz(*inBlock, nullptr);
          if(deathBlitzReturn != nullptr){
            //errs()<<"!!!!!!!!! FOUND !!!!!!!!!!!!!!!\n";
            ConstantInt * aBaseCaseArg = dyn_cast<ConstantInt>((Value *)deathBlitzReturn);
            baseCaseArg.push_back(dyn_cast<ConstantInt>(inValue)->getSExtValue());
            baseCaseVal.push_back(aBaseCaseArg->getSExtValue());
            errs()<<"BASE CASE VALUE: ";
            aBaseCaseArg->dump();
          }
          else{
            errs()<<"XXXXXXXXXX BASE CASE NOT FOUND XXXXXXXXXX\n";
            assert(0);
          }
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
