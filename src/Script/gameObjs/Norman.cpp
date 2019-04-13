/*
 * ========================= Norman.cpp ==========================
 *                          -- tpr --
 *                                        CREATE -- 2019.01.30
 *                                        MODIFY -- 
 * ----------------------------------------------------------
 */
#include "Script/gameObjs/Norman.h"

//-------------------- C --------------------//
#include <cassert> //- assert

//-------------------- CPP --------------------//
#include <functional>
#include <string>

//-------------------- Engine --------------------//
#include "srcs_engine.h" 

//-------------------- Script --------------------//
#include "Script/resource/srcs_script.h" 

using namespace std::placeholders;

#include "debug.h" 


namespace gameObjs{//------------- namespace gameObjs ----------------


//namespace{//-------------- namespace ------------------//
//}//------------------ namespace: end ------------------//


/* ===========================================================
 *                         init
 * -----------------------------------------------------------
 */
void Norman::init( GameObj *_goPtr ){

    assert( _goPtr != nullptr );
    goPtr = _goPtr;

    //-------- bind callback funcs ---------//
    //-- 故意将 首参数this 绑定到 保留类实例 dog_a 身上
    goPtr->RenderUpdate = std::bind( &Norman::OnRenderUpdate, &norman, _1 );   
    goPtr->LogicUpdate  = std::bind( &Norman::OnLogicUpdate,  &norman, _1 );
    
    //-------- actionSwitch ---------//
    goPtr->actionSwitch.bind_func( std::bind( &Norman::OnActionSwitch, &norman, _1, _2 ) );
    goPtr->actionSwitch.signUp( ActionSwitchType::Move_Idle );
    goPtr->actionSwitch.signUp( ActionSwitchType::Move_Move );


    //-------- go self vals ---------//
    goPtr->species = Norman::specId;
    goPtr->family = GameObjFamily::Major;
    goPtr->parentId = NULLID;
    goPtr->state = GameObjState::Waked;
    goPtr->moveState = GameObjMoveState::Movable;
    goPtr->weight = 5.0f;

    goPtr->isTopGo = true;
    goPtr->isActive = true;
    goPtr->isDirty = false;
    goPtr->isControlByPlayer = false;

    //goPtr->move.set_speedLv( SpeedLevel::LV_6 ); //- 标准crawl速度 4/5/6 都不错
    goPtr->move.set_speedLv( SpeedLevel::LV_8 );   //- tmp，用来快速检索地图
    goPtr->move.set_MoveType( true ); //- tmp

    goPtr->goPos.set_alti( 0.0f );

    goPtr->set_collision_isDoPass( false );
    goPtr->set_collision_isBePass( false );

    //-------- animFrameSet／animFrameIdxHandle/ goMesh ---------//

        //-- 制作唯一的 mesh 实例: "root" --
        GameObjMesh &rootGoMeshRef = goPtr->creat_new_goMesh( "root", "norman" );
        rootGoMeshRef.init( goPtr ); 
        rootGoMeshRef.set_pic_renderLayer( RenderLayerType::MajorGoes ); //- 不设置 固定zOff值
        rootGoMeshRef.picMesh.set_shader_program( &esrc::rect_shader );
        rootGoMeshRef.shadowMesh.set_shader_program( &esrc::rect_shader ); //- 没有 shadow 时不用设置
        //-- bind animFrameSet / animFrameIdxHandle --
        rootGoMeshRef.animFrameIdxHandle.bind_cycle(0,   //- 起始图元帧序号
                                                5,   //- 结束图元帧序号
                                                0,   //- 入口图元帧序号  
                                                true //- isOrder
                                                );

        rootGoMeshRef.isVisible = true;
        rootGoMeshRef.isCollide = true;
        rootGoMeshRef.isFlipOver = false;

        //-- goMesh pos in go --
        rootGoMeshRef.pposOff = glm::vec2{ 0.0f, 0.0f }; //- 此 goMesh 在 go 中的 坐标偏移 
        rootGoMeshRef.off_z = 0.0f;  //- 作为 0号goMesh,此值必须为0

    //-------- go.pvtBinary ---------//
    goPtr->resize_pvtBinary( sizeof(Norman_PvtBinary) );
    pvtBp = (Norman_PvtBinary*)goPtr->get_pvtBinaryPtr(); //- 绑定到本地指针

    
    //...

    //-------- go.pubBinary ---------//
    goPtr->pubBinary.init( norman_pubBinaryValTypes );
}

/* ===========================================================
 *                       bind
 * -----------------------------------------------------------
 * -- 在 “工厂”模式中，将本具象go实例，与 一个已经存在的 go实例 绑定。
 * -- 这个 go实例 的类型，应该和 本类一致。
 */
void Norman::bind( GameObj *_goPtr ){
}


/* ===========================================================
 *                       rebind
 * -----------------------------------------------------------
 * -- 从硬盘读取到 go实例数据后，重bind callback
 * -- 会被 脚本层的一个 巨型分配函数 调用
 */
void Norman::rebind( GameObj *_goPtr ){
}

/* ===========================================================
 *                      OnRenderUpdate
 * -----------------------------------------------------------
 */
void Norman::OnRenderUpdate( GameObj *_goPtr ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    rebind_ptr( _goPtr );

    //=====================================//
    //            AI
    //-------------------------------------//
    //...

    //=====================================//
    //         更新 位移系统
    //-------------------------------------//
    goPtr->move.RenderUpdate();

    //=====================================//
    //  将 确认要渲染的 goMeshs，添加到 renderPool         
    //-------------------------------------//
    for( auto &pairRef : goPtr->goMeshs ){
        pairRef.second.RenderUpdate();
    }
}


/* ===========================================================
 *                        OnLogicUpdate
 * -----------------------------------------------------------
 */
void Norman::OnLogicUpdate( GameObj *_goPtr ){
    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    rebind_ptr( _goPtr );
    //=====================================//

    // 什么也没做...
}



/* ===========================================================
 *               OnActionSwitch
 * -----------------------------------------------------------
 * -- 此处用到的 animFrameIdxHdle实例，是每次用到时，临时 生产／改写 的
 * -- 会被 动作状态机 取代...
 */
void Norman::OnActionSwitch( GameObj *_goPtr, ActionSwitchType _type ){

    //=====================================//
    //            ptr rebind
    //-------------------------------------//
    rebind_ptr( _goPtr );
    //=====================================//

    //-- 获得所有 goMesh 的访问权 --
    GameObjMesh &rootGoMeshRef = goPtr->goMeshs.at("root");

    //-- 处理不同的 actionSwitch 分支 --
    switch( _type ){
        case ActionSwitchType::Move_Idle:
            //rootGoMeshRef.bind_animFrameSet( "norman" );
            rootGoMeshRef.animFrameIdxHandle.bind_cycle(0,   //- 起始图元帧序号
                                                        5,   //- 结束图元帧序号
                                                        0,   //- 入口图元帧序号  
                                                        true //- isOrder
                                                        );
            break;

        case ActionSwitchType::Move_Move:
            //rootGoMeshRef.bind_animFrameSet( "norman" );
            rootGoMeshRef.animFrameIdxHandle.bind_cycle(6,   //- 起始图元帧序号
                                                        11,  //- 结束图元帧序号
                                                        6,   //- 入口图元帧序号  
                                                        true //- isOrder
                                                        );
            break;

        default:
            break;
            //-- 并不报错，什么也不做...

    }


}

}//------------- namespace gameObjs: end ----------------

