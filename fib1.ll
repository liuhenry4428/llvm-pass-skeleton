; ModuleID = 'fib.ll'
source_filename = "fib.c"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-macosx10.15.0"

@.str = private unnamed_addr constant [9 x i8] c"Fib7: %i\00", align 1

; Function Attrs: noinline nounwind ssp uwtable
define i32 @fibbonacci(i32) #0 {
  %.reg2mem = alloca i32
  %.0.reg2mem = alloca i32
  %"reg2mem alloca point" = bitcast i32 0 to i32
  %2 = icmp eq i32 %0, 0
  br i1 %2, label %3, label %4

3:                                                ; preds = %1
  store i32 0, i32* %.0.reg2mem
  br label %13

4:                                                ; preds = %1
  %5 = icmp eq i32 %0, 1
  br i1 %5, label %6, label %7

6:                                                ; preds = %4
  store i32 1, i32* %.0.reg2mem
  br label %13

7:                                                ; preds = %4
  %8 = sub nsw i32 %0, 1
  %9 = call i32 @fibbonacci(i32 %8)
  %10 = sub nsw i32 %0, 2
  %11 = call i32 @fibbonacci(i32 %10)
  %12 = add nsw i32 %9, %11
  store i32 %12, i32* %.reg2mem
  %.reload = load i32, i32* %.reg2mem
  store i32 %.reload, i32* %.0.reg2mem
  br label %13

13:                                               ; preds = %7, %6, %3
  %.0.reload = load i32, i32* %.0.reg2mem
  ret i32 %.0.reload
}

; Function Attrs: noinline nounwind ssp uwtable
define i32 @main() #0 {
  %"reg2mem alloca point" = bitcast i32 0 to i32
  %1 = call i32 @fibbonacci(i32 7)
  %2 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @.str, i32 0, i32 0), i32 %1)
  ret i32 0
}

declare i32 @printf(i8*, ...) #1

attributes #0 = { noinline nounwind ssp uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "darwin-stkchk-strong-link" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "probe-stack"="___chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="penryn" "target-features"="+cx16,+fxsr,+mmx,+sahf,+sse,+sse2,+sse3,+sse4.1,+ssse3,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1, !2}
!llvm.ident = !{!3}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 10, i32 15]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 7, !"PIC Level", i32 2}
!3 = !{!"Apple clang version 11.0.0 (clang-1100.0.33.12)"}
