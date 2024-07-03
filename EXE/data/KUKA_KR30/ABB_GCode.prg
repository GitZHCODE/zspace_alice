
 MODULE Module1 

 !Flag, end the program 
 
 VAR bool bProgEnd; 
 
 !Constant for the joint calibrate position 
 
 CONST jointtarget calib_pos := [[0, 0, 0, 0, 0, 0], [0, 9E9, 9E9, 9E9, 9E9, 9E9]]; 

 PROC Main() 
 
 Init; 
 
 mv_Calib; 
 
 mv_Custom; 
 
 mv_Calib; 
 
 ENDPROC 
 
 PROC Init() 
 
 !Defined setting of the variables 

 bProgEnd := FALSE; 
 
 ENDPROC 
 
 PROC mv_Calib() 
 
 MoveAbsJ calib_pos,v100,z10,tool0; 
 
 ENDPROC 
 
 PROC mv_Custom() 
 
  CONST num count := 0;
  CONST jointtarget poses {count} := 
 [ 
 ]; 
 FOR i FROM 1 TO count DO 
 MoveAbsJ poses{i}, v100, z10, tool0;
 ENDFOR
 ENDPROC 
 
 ENDMODULE 
