import mqtt from "../../utils/mqtt.min";
const MQTTADDRESS = "你的mqtt服务器地址"; //mqtt服务器地址
let client = null; //mqtt服务

Page({

  /**
   * 页面的初始数据
   */
  data: {
      title:"图书管理系统",
      result : "",
      switchStatus: false,

      isConnect: false, //是否连接
      mqttContontDialog: false, //mqtt打开连接打开弹窗
      isPush: false, //是否订阅
      isSubscr: false, //是否添加发布地址

      /* 连接输入框 */
		  address: wx.getStorageSync('address') || '',
	  	port: wx.getStorageSync('port') || '',
		  username: wx.getStorageSync('username') || '',
		  password: wx.getStorageSync('password') || '',

		  push: wx.getStorageSync('push') || '', //订阅地址
      subscr: wx.getStorageSync('subscr') || '', //发布地址

      angle: wx.getStorageSync('angle') || '', //角度

    /* 图书信息 */
    location1: "00-0",
    islocation1:true,

    location2: "00-0",
    islocation2:true,

    location3: "00-0",
    islocation3:true,

    location4: "00-0",
    islocation4:true,

    location5: "00-0",
    islocation5:true,

    //其他设备
    otherSensorList: [
			{
				img: "/images/car.jpg",
        name: "小车",
        isOpen: false
			},
    ], 
    otherSensorList2: [  
    {
      img: "/images/camera2.jpg",
      name: "摄像头",
      isOpen: false
    }],
    otherSensorList3: [  
      {
        img: "/images/er.jpg",
        name: "二维码：",
      }],
    otherSensorList4: [  
      {
        img: "/images/duoji.jpg",
        name: "舵机",
      }]
  },

  //信息处理
  deal_message(){
    let that = this;
    that.setData({islocation1:true});
    that.setData({location1:"00-0"});
    that.setData({islocation2:true});
    that.setData({location2:"00-0"});
    that.setData({islocation3:true});
    that.setData({location3:"00-0"});
    that.setData({islocation4:true});
    that.setData({location4:"00-0"});
    that.setData({islocation5:true});
    that.setData({location5:"00-0"});
  },

	/* 打开连接弹窗 */
	openDialog() {
		this.setData({
			mqttContontDialog: true
		});
  },

  /* 关闭连接弹窗 */
  onClose() {
		this.setData({
			mqttContontDialog: false
		});
	},
  
  // MQTT 连接
  connectMqtt() {
    console.log(this.data.address,
      this.data.port,
      this.data.username,
      this.data.password,
      );
    let that = this;
    const options = {
      connectTimeout: 4000,
      address: this.data.address, // 输入的连接地址
      port: this.data.port, // 输入的端口号
      username: this.data.username, // 输入的用户名
      password: this.data.password, // 输入的密码
    };
    console.log("address是：", options.address);
    client = mqtt.connect("wxs://" + options.address + "/mqtt", options); // 连接方法
    client.on("connect", error => {
      console.log("连接成功");
      // 可以在这里写一些连接成功的逻辑
      this.setData({
				isConnect: true
      })
      wx.setStorageSync('address', options.address)
			wx.setStorageSync('port', options.port)
			wx.setStorageSync('username', options.username)
			wx.setStorageSync('password', options.password)
      client.on("message", (topic, message) => {
      console.log("收到消息：", message.toString());
      let getMessageObj = {}; //收到的消息
      getMessageObj = message.toString(); //收到的消息转换成文本对象
      console.log(getMessageObj);
      if(getMessageObj == "01-1")
      {
        that.setData({islocation1:false});
        that.setData({location1:"01-1"});
      }
      if(getMessageObj == "01-2")
      {
        that.setData({islocation1:false});
        that.setData({location1:"01-2"});
      }
      if(getMessageObj == "01-3")
      {
        that.setData({islocation1:false});
        that.setData({location1:"01-3"});
      }
      if(getMessageObj == "02-1")
      {
        that.setData({islocation2:false});
        that.setData({location2:"02-1"});
      }
      if(getMessageObj == "02-2")
      {
        that.setData({islocation2:false});
        that.setData({location2:"02-2"});
      }
      if(getMessageObj == "02-3")
      {
        that.setData({islocation2:false});
        that.setData({location2:"02-3"});
      }
      if(getMessageObj == "03-1")
      {
        that.setData({islocation3:false});
        that.setData({location3:"03-1"});
      }
      if(getMessageObj == "03-2")
      {
        that.setData({islocation3:false});
        that.setData({location3:"03-2"});
      }
      if(getMessageObj == "03-3")
      {
        that.setData({islocation3:false});
        that.setData({location3:"03-3"});
      }
      if(getMessageObj == "04-1")
      {
        that.setData({islocation4:false});
        that.setData({location4:"04-1"});
      }
      if(getMessageObj == "04-2")
      {
        that.setData({islocation4:false});
        that.setData({location4:"04-2"});
      }
      if(getMessageObj == "04-3")
      {
        that.setData({islocation4:false});
        that.setData({location4:"04-3"});
      }
      if(getMessageObj == "没有错书")
      {
        that.setData({islocation5:false});
        that.setData({location5:"这个书籍区没有错书"});
      }
      })
    });

    client.on("reconnect", error => {
      console.log("正在重连：", error);
      wx.showToast({ icon: "none", title: "正在重连", });
    });
    client.on("error", error => {
      console.log("连接失败：", error);
      wx.showToast({ icon: "none", title: "MQTT连接失败", });
    });
  },

  // 关闭连接
	closeConnect() {
		client.end();
		this.setData({
			isConnect: false,
			isPush: false,
		})
  },
  
  /* 添加订阅 */
	addPush() {
		let that = this
		//订阅一个主题
		if (!this.data.isConnect) {
			wx.showToast({
				icon: "none",
				title: "请先连接",
			});
			return
		}
		client.subscribe(this.data.push, {
			qos: 0
		}, function (err) {
			if (!err) {
				console.log("订阅成功");
				wx.showToast({
					icon: "none",
					title: "订阅成功",
				});
				that.setData({
					isPush: true
				})
				wx.setStorageSync('push', that.data.push)
			}
		});
  },
  
  // 取消订阅
  closePush() {
		let that = this
		client.unsubscribe(this.data.push, function (err) {
			if (!err) {
				wx.showToast({
					icon: "none",
					title: "取消成功",
				});
				that.setData({
					isPush: false
				})
			}
		});
  },

  /* 添加发布 */
  addSubscr() {
		if (!this.data.isConnect) {
			wx.showToast({
				icon: "none",
				title: "请先连接",
			});
			return
		}
		let that = this
		client.subscribe(this.data.subscr, {
			qos: 0
		}, function (err) {
			if (!err) {
				// 发布消息
				console.log('添加成功');
				that.setData({
					isSubscr: true
				})
				wx.setStorageSync('subscr', that.data.subscr)
				// console.log("发出的", msg);
				// client.publish(this.data.subscr, JSON.stringify(msg)); //转换json格式
			}
		});
  },

  systemChange(e) {
		let that = this
		let clickData = e.target.dataset.param
		let value = e.detail.value
		let msg

		if (clickData.name === '小车') {
			if (!value) 
				msg = "stop"
			else 
				msg = "setup"		
		}
		if (clickData.name === '摄像头') {
      if(value)
      {
        const ctx = wx.createCameraContext()
        ctx.takePhoto({
          quality: 'high',
          success: (res) => {
            this.setData({
              src: res.tempImagePath
            })
          }
        })
      }

		}
		client.subscribe(this.data.subscr, {
			qos: 0
		}, function (err) {
			if (!err) {
				console.log("发出的", msg);
			client.publish(that.data.subscr, msg); //转换json格式
			}
		});
	},

  angle() {
		let that = this
    let msg
    
    msg = this.data.angle
		client.subscribe(this.data.subscr, {
			qos: 0
		}, function (err) {
			if (!err) {
				console.log("发出的", msg);
			client.publish(that.data.subscr, msg); //转换json格式
			}
		});
  },
  
  systemChange2(e) {
		let that = this
		let clickData = e.target.dataset.param
		let value = e.detail.value

		if (clickData.name === '摄像头') {
      if(value)
      {
        wx.scanCode({
          success: (res) => {
            console.log('扫描结果:', res.result);
            that.setData({result:res.result})
            // 根据扫描结果执行相应逻辑
          },
          fail: (err) => {
            console.error('扫描失败:', err);
          }
        });
      }

      setTimeout(() => {
        this.setData({
          switchStatus: false
        });
      }, 3000)
		}
	},
})