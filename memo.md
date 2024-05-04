# ROS节点

<div align="center"><img src="img/rosgraph.png" width=600px/></div>

## 建图模式使用的节点
* **ssl_slam2_laser_processing_node**: 
    * 订阅消息: /camera/depth/color/points
    * 发布消息：
        * /velodyne_points_filtered: 其他节点并未使用该消息
        * /laser_cloud_edge：边缘特征
        * /laser_cloud_surf：平面特征
* **ssl_slam2_odom_estimation_mapping_node**: 利用提取的平面和边缘特征估计相机的运动
* **ssl_slam2_map_optimization_node**: 


