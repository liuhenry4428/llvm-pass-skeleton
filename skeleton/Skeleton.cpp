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
#include <unordered_map>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include <iostream> 
// #include "llvm/Core.h"


using namespace llvm;


Function * buildMemoized(
  std::vector<int> baseCaseArg, 
  std::vector<int> baseCaseVal, 
  std::vector<int64_t> constOffsets,
  std::vector<Instruction *> recCallReturns,
  std::vector<Instruction *> dependents,
  std::vector<Value *> undefCallReturns,
  std::unordered_map<Use *, int> useToOffset,
  Function * theWholeFunction,
  LLVMContext & context){
  errs()<<"<<BEGIN BUILD>>\n";
  // Module * theModule = new Module("ourModule", context);
  // auto * theModule = theWholeFunction->getParent();
  // auto funcCallee = theModule->getOrInsertFunction(theWholeFunction->getName(), theWholeFunction->getFunctionType());
  // auto * func = theModule->getFunction(theWholeFunction->getName());
  auto func = theWholeFunction;
  for(auto thing : dependents){
    thing->dump();
  }

  // auto func = Function(theWholeFunction->getFunctionType(), theWholeFunction->getLinkage(), theWholeFunction->getAddressSpace(), "myNewFunc", modulePtr);
  // auto func = module.getOrInsertFunction("myNewFunc", theWholeFunction->getFunctionType());

  Argument * funcArg = func->arg_begin();

  //TODO refactor
  
  BasicBlock * nextNewBlock = BasicBlock::Create(context, "BaseCaseStuff", func);
  errs()<<"<<BEGIN BASE CASES>>\n";
  IRBuilder<> builder(context);
  for(int i = 0; i<baseCaseArg.size(); i++){
    builder.SetInsertPoint(nextNewBlock);
    auto * trueBlock = BasicBlock::Create(context, "TrueBlock", func);
    auto * falseBlock = BasicBlock::Create(context, "FalseBlock", func);
    auto * myCond = builder.CreateICmpEQ(ConstantInt::get(funcArg->getType(), baseCaseArg.at(i)), funcArg);
    auto * myBranch = builder.CreateCondBr(myCond, trueBlock, falseBlock, (Instruction *)nullptr);
    builder.SetInsertPoint(trueBlock);
    builder.CreateRet(ConstantInt::get(funcArg->getType(), baseCaseVal.at(i)));
    nextNewBlock = falseBlock;
    builder.SetInsertPoint(nextNewBlock);
  }

  errs()<<"<<BEGIN HEADER>>\n";
  // Iterative case
  std::vector<AllocaInst *>iPtrs;
  builder.SetInsertPoint(nextNewBlock);
  auto * theStupidType = funcArg->getType();

  auto itVal = builder.CreateAdd(ConstantInt::get(theStupidType, baseCaseArg.back()), ConstantInt::get(theStupidType, 0), "iteratorVal");
  auto itPtr = builder.CreateAlloca(theStupidType,nullptr, "itPtr" );
  builder.CreateStore(itVal, itPtr);
  int numMemoValues = recCallReturns.size();
  for(int i = 0; i<numMemoValues; i++){
    auto iVal = builder.CreateAdd(ConstantInt::get(theStupidType, baseCaseVal.at(i)), ConstantInt::get(theStupidType, 0), "iVal");
    auto iPtr = builder.CreateAlloca(theStupidType,nullptr, "iPtr" );
    iPtrs.push_back(iPtr);
    builder.CreateStore(iVal, iPtr);
  }

  errs()<<"<<BEGIN BODY -- LOADING>>\n";
  auto * WhileBody = BasicBlock::Create(context, "WhileBody", func);
  builder.CreateBr(WhileBody);


  // //Do While Body
  std::vector<LoadInst *>loadedValues;
  builder.SetInsertPoint(WhileBody);
  auto itLoaded = builder.CreateLoad(itPtr, "itLoaded");
  for(int i = 0; i<numMemoValues; i++){ // TODO SHOULD USE MAX(CONSTNAT OFFSETS) NOT recCallReturns.size() !!!
    auto iLoaded = builder.CreateLoad(iPtrs.at(i), "iLoaded"+itostr(i));
    loadedValues.push_back(iLoaded);
    //undefCallReturns.at(i)->replaceAllUsesWith(iLoaded);
  }
  errs()<<"<<BEGIN BODY -- DEPENDENTS>>\n";
  Instruction * lastInst;
  for(auto * depInst : dependents){ //TODO make dependents vector<Instruction>
    for(auto depOp = depInst->op_begin(); depOp != depInst->op_end(); ++depOp){
      if(useToOffset.count(depOp)){
        Instruction * newIshit = loadedValues.at(numMemoValues-useToOffset[depOp]);
        depOp->set(newIshit);
        depInst->dump();
      }
      // int watafuck = std::find(recCallReturns.begin(), recCallReturns.end(), depOp->get()) - recCallReturns.begin();
    }
    WhileBody->getInstList().push_back(dyn_cast<Instruction>(depInst));
    lastInst = depInst;
  }

  errs()<<"<<BEGIN BODY -- STOREBACKs>>\n";
  // auto current = builder.CreateAdd(i1Loaded, i2Loxaded, "current");
  // auto current = WhileBody->getInstList().rbegin(); //fuck this
  for(int i=0; i<recCallReturns.size(); i++){
    builder.CreateStore((i == recCallReturns.size()-1) ? lastInst : loadedValues.at(i+1), iPtrs.at(i));
  }

  errs()<<"<<BEGIN BODY -- GUARD AND SHIT>>\n";
  
  // builder.CreateStore(current, i2Ptr);
  auto increment = builder.CreateAdd(itLoaded,ConstantInt::get(theStupidType, 1), "increment");
  builder.CreateStore(increment, itPtr);

  //auto itLoaded2 = builder.CreateLoad(itPtr, "itLoadedAgain");
  auto loopGuard = builder.CreateICmpSLT(increment, funcArg, "loopGuard");
  auto returnBlock = BasicBlock::Create(context, "returnBlock", func);
  builder.CreateCondBr(loopGuard, WhileBody, returnBlock);

  errs()<<"<<BEGIN RETURN>>\n";
  builder.SetInsertPoint(returnBlock);
  builder.CreateRet(lastInst);

  errs()<<"-------------------------------------------------\n";
  func->dump();
  errs()<<"-------------------------------------------------\n";
  return func;
}


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
      //TODO fix scope
      std::vector<Instruction *> recCallReturns;
      std::vector<Instruction *> dependents;
      std::vector<Value *> undefCallReturns;
      std::unordered_map<Value *, int> recCallToOffset;
      std::unordered_map<Use *, int> useToOffset;
      // std::unordered_map<Value *, Value *> originalDepToClone;

      for (auto &bb : F) {
        for (auto &instruction : bb) {
          // errs()<<"REC CALL TO OFFSET SIZE(): " << recCallToOf fset.size();
          //If inst is dependent on recursive call return
          if(Instruction * someInst = dyn_cast<Instruction>(&instruction)){
            if((recCallReturns.size() != 0 )&& (recCallReturns.at(0)->getParent() != &bb)) continue; //Bandaid over a bullet hole
            bool exitFlag = false;//prevents double counting
            for(auto useIt = someInst->op_begin(); useIt != someInst->op_end(); ++useIt){
              auto * currentUse = useIt->get();
              
              //If dependent on rec call
              for(auto dep : dependents){
                if(dep == currentUse){
                  auto clonedInst = someInst->clone();
                  someInst->replaceAllUsesWith(clonedInst);
                  for(auto  smallOp = clonedInst->op_begin(); smallOp != clonedInst->op_end(); ++smallOp) {
                    errs()<<"smallOp->get()\n";
                    smallOp->get()->dump();
                    if(recCallToOffset.count(smallOp->get())) {
                      int offsetVal = recCallToOffset[*smallOp]; 
                      useToOffset[smallOp] = offsetVal; 
                      // errs()<<"OFFSET VAL: "<<offsetVal; 
                      smallOp->set(UndefValue::get(smallOp->get()->getType()));} 
                  }
                  clonedInst->setName("clonedName");
                  dependents.push_back(clonedInst);
                  // dependents.push_back(someInst);
                  exitFlag = true; 
                  break;
                }
              }
              if(exitFlag)break;
              //If dependent on something dependent on a rec call
              // errs()<<"RECCALLRETURNS SIZE():\n"<< recCallReturns.size();
              for(auto rcr : recCallReturns){
                if(rcr == currentUse){
                  errs()<<"DOING INST:\n";
                  someInst->dump();
                  auto clonedInst = someInst->clone();
                  someInst->replaceAllUsesWith(clonedInst);
                  for(auto  smallOp = clonedInst->op_begin(); smallOp != clonedInst->op_end(); ++smallOp) {
                    errs()<<"smallOp->get()\n";
                    smallOp->get()->dump();
                    if(recCallToOffset.count(smallOp->get())) {
                      int offsetVal = recCallToOffset[*smallOp]; 
                      useToOffset[smallOp] = offsetVal; 
                      // errs()<<"OFFSET VAL: "<<offsetVal; 
                      smallOp->set(UndefValue::get(smallOp->get()->getType()));} 
                  }
                  clonedInst->setName("clonedName");
                  dependents.push_back(clonedInst);
                  // dependents.push_back(someInst);
                  exitFlag = true;
                  break;
                }
              }
              // if(exitFlag)break;
              // //If dependent on something dependent on a rec call
              // for(auto ucr : undefCallReturns){
              //   if(ucr == currentUse){
              //     auto clonedInst = someInst->clone();
              //     someInst->replaceAllUsesWith(clonedInst);
              //     for(auto  smallOp = clonedInst->op_begin(); smallOp != clonedInst->op_end(); ++smallOp) {
              //       if(recCallToOffset.count(smallOp->get())) {int offsetVal = recCallToOffset[*smallOp]; useToOffset[smallOp] = offsetVal; errs()<<"OFFSET VAL: "<<offsetVal; smallOp->set(UndefValue::get(smallOp->get()->getType()));} 
              //       // else if(originalDepToClone.count(smallOp->get())) {smallOp->set(originalDepToClone[smallOp->get()]);}
              //       // else assert (0);
              //     }
              //     clonedInst->setName("clonedName");
              //     dependents.push_back(clonedInst);
              //     // dependents.push_back(someInst);
              //     exitFlag = true;
              //     break;
              //   }
              // }
            }
            if(exitFlag) continue;
          }

          if (CallBase *callInst = dyn_cast<CallBase>(&instruction)) {
            if (Function *calledFunction = callInst->getCalledFunction()) {
              if (calledFunction->getName().startswith(F.getName())) {
                callInst->setName("recCallReturn");
                recCallReturns.push_back(callInst);
                auto newUndef = UndefValue::get(callInst->getType());
                // auto newUndef = CallInst::Create(callInst->getFunctionType(), &F, "undefCall");
                // undefCallReturns.push_back(newUndef);
                //callInst->replaceAllUsesWith(newUndef);
                for(auto recArg = callInst->arg_begin(); recArg != callInst->arg_end(); ++recArg) { //should only need 1 iteration
                  auto* recArgValue =recArg->get();
                  recArgValue->setName("recArg");
                  if(auto * recArgInst = dyn_cast<Instruction>(recArgValue)){
                    if(auto * constArg = dyn_cast<ConstantInt>(recArgInst->getOperand(1))){
                      if(recArgInst->getOperand(0)->getName().startswith("funcArg")){
                        constOffsets.push_back(constArg->getSExtValue());
                        recCallToOffset[callInst] = constArg->getSExtValue();
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
          if (returnStmt == NULL) returnStmt = retInst; else return false;
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

      errs()<<"BEGIN ERASE!!! \n <><><><><><><><><><><><>\n";

      F.dropAllReferences();
      F.deleteBody();
      errs()<<"<<ABOUT TO BEGIN BUILD>>\n";
      Function * memoized = buildMemoized(baseCaseArg, baseCaseVal, constOffsets, recCallReturns, dependents,undefCallReturns, useToOffset,&F, F.getContext());
      
      return true;
    }

  };
}









char SkeletonPass::ID = 0;

// Register the pass so `opt -skeleton` runs it.
static RegisterPass<SkeletonPass> X("skeleton", "a useless pass");
