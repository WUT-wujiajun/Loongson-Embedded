import cv2
from pyzbar.pyzbar import decode
import paho.mqtt.client as mqtt
import time

# MQTT Broker地址和端口
broker_address = "esp.icce.top"
broker_port = 1883

# MQTT发布主题
wrong_book_topic = "WrongBook"
view_topic = "view"
location_topic = "location"

# 初始化摄像头
cap = cv2.VideoCapture(1)

# 初始化列表和字典，all_qrcodes用于存储5个二维码存储信息，qr_positions用于存储相应data对应的二维码中心位置
all_qrcodes = []
qr_positions = {}

view_flag = False


# MQTT连接回调函数
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker")
    else:
        print("Failed to connect, return code: ", rc)


# MQTT发布消息回调函数
def on_publish(client, userdata, mid):
    print("Message published")

# MQTT消息回调函数
def on_message(client, userdata, msg):
    global view_flag
    if msg.topic == view_topic and msg.payload.decode("utf-8") == "开始视觉识别":
        print("接收到'开始视觉识别'消息,等待2秒后进行识别操作...")
        time.sleep(2)

        view_flag = True
        print("标志位变true")

# 创建MQTT客户端并设置回调函数
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.on_publish = on_publish

# 连接到MQTT Broker
client.connect(broker_address, broker_port, 60)
# 启动MQTT事件循环
client.loop_start()
# 启动MQTT消息监听
client.subscribe(view_topic)

# 等待连接成功
while not client.is_connected():
    pass

while True:
    # 读取摄像头帧
    ret, frame = cap.read()
    if not ret:
        break

    # 使用pyzbar解码二维码
    qrcodes = decode(frame)

    # 清空位置字典和数据列表
    qr_positions.clear()
    all_qrcodes.clear()

    # 遍历二维码
    for qrcode in qrcodes:
        data = qrcode.data.decode("utf-8")
        left, top, width, height = qrcode.rect.left, qrcode.rect.top, qrcode.rect.width, qrcode.rect.height
        center = (int(left + width / 2), int(top + height / 2))  # 计算二维码的中心点(x,y)坐标
        qr_positions[data] = center

        if data not in all_qrcodes:
            all_qrcodes.append(data)

    # 在图像上绘制识别到的二维码边界框和文本
    for qrcode in qrcodes:
        x, y, w, h = qrcode.rect
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, qrcode.data.decode("utf-8"), (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0, 255, 0), 2)
    # 检查是否所有二维码都已识别
    if len(all_qrcodes) <= 5 and view_flag:
        # 查找01-L和01-R的中心位置
        location = '00-R'
        # data是键，pos是值
        for data, pos in qr_positions.items():
            if data == '01-R':
                location = '01-'
            elif data == '02-R':
                location = '02-'
            elif data == '03-R':
                location = '03-'
            elif data == '04-R':
                location = '04-'
            print(f"{location}")

        # 根据01-L和01-R的位置对中间三个二维码进行排序
        # 使用二维向量的点积和叉积来确定顺序可能不准确，因为点积和叉积在二维空间中不提供足够的排序信息
        # 我们可以简单地基于x坐标进行排序，假设摄像头是从左到右拍摄的
        sorted_books = sorted(qr_positions.items(), key=lambda x: x[1][0])  # center中的x坐标

        # 提取中间三个二维码的数据，并检查是否放错
        wrong_books = []
        for i, (data, _) in enumerate(sorted_books[1:4], 1):  # 中间三个二维码
            if not data.startswith(location):
                wrong_books.append((i, data))
        # 输出结果
        if view_flag and location != '00-R':
            if wrong_books:
                for idx, book_info in wrong_books:
                    message = f"{idx}"
                    client.publish(wrong_book_topic, message)
                    client.publish(wrong_book_topic, message)
                    client.publish(wrong_book_topic, message)
                    message2 = f"{location}{idx}"
                    client.publish(location_topic, message2)
                    view_flag = False
                    print(f"错书：从左到右第{idx}本书：{book_info}")
                    client.publish(view_topic, "视觉识别完成")
            else:
                print(f"没有错书")
                view_flag = False
                client.publish(wrong_book_topic, "没有错书")
                client.publish(view_topic, "视觉识别完成")
    # 显示图像
    cv2.imshow("QR Code Detection", frame)

    # 按'Esc'退出
    key = cv2.waitKey(1) & 0xFF
    if key == 27:  # 'Esc' 键
        client.loop_stop()
        client.disconnect()
        cap.release()
        cv2.destroyAllWindows()
