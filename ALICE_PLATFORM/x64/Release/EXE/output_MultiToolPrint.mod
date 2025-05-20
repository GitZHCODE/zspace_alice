MODULE MultiToolPrint
    PERS wobjdata aiSyncWObj := [FALSE,TRUE,"",[[1250.260000,-572.790000,-684.000000],[0.999992,0.000332,-0.004107,0.000000]],[[0.000000,0.000000,0.000000],[0.999936,-0.010035,0.005236,-0.000053]]];
    PERS tooldata aiSyncTool := [TRUE, [[325.451000,2.555000,91.138000], [0.707107,0.000000,-0.707107,0.000000]], [25.000000, [0.000000,0.000000,0.000000], [0.999936,-0.010035,0.005236,-0.000053], 0.000000, 0.000000, 0.000000]];
    PERS tooldata standardTool := [TRUE, [[325.451000,2.555000,91.138000], [0.707107,0.000000,-0.707107,0.000000]], [25.000000, [0.000000,0.000000,0.000000], [0.999936,-0.010035,0.005236,-0.000053], 0.000000, 0.000000, 0.000000]];
    PERS tooldata fineTool := [TRUE, [[325.451000,2.555000,81.138000], [0.707107,0.000000,-0.707107,0.000000]], [25.000000, [0.000000,0.000000,0.000000], [0.999936,-0.010035,0.005236,-0.000053], 0.000000, 0.000000, 0.000000]];
    PERS zonedata aiSyncZone := [FALSE, 0.100000, 0.100000, 0.100000, 0.010000, 0.100000, 0.010000];
    PERS zonedata preciseZone := [FALSE, 0.050000, 0.050000, 0.100000, 0.010000, 0.100000, 0.010000];
    PERS zonedata roughZone := [FALSE, 0.200000, 0.200000, 0.100000, 0.010000, 0.100000, 0.010000];
    PERS confdata aiSyncAxis := [0, -1, 0, 1];

    PROC Main()
        !LAYER 0
        !LINETYPE OUTER_WALL
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[200, 200, 5.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[200, 200, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 1;
        SetAO aoExtrRate_ZH, 46.068240;
        MoveL [[200, 200, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        MoveL [[300, 200, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        MoveL [[300, 300, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        MoveL [[200, 300, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        MoveL [[200, 200, 0.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[200, 200, 5.2], [0.999936, -0.010035, 0.005236, -5.3e-05], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], roughZone, standardTool, \Wobj:=aiSyncWObj;
        !LINETYPE INNER_WALL
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[210, 210, 5.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[210, 210, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 1;
        SetAO aoExtrRate_ZH, 46.068240;
        MoveL [[210, 210, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        MoveL [[290, 210, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        MoveL [[290, 290, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        MoveL [[210, 290, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        MoveL [[210, 210, 0.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [20, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
        SetDO doStartExtrusion_ZH, 0;
        SetAO aoExtrRate_ZH, 0.000000;
        MoveL [[210, 210, 5.2], [0.707107, 0, -0.707107, 0], aiSyncAxis, [9E9, 9E9, 9E9, 9E9, 9E9, 9E9]], [100, 500, 5000, 1000], preciseZone, fineTool, \Wobj:=aiSyncWObj;
    ENDPROC

ENDMODULE
