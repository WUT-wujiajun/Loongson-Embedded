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
cap = cv2.VideoCapture(0)

# 初始化列表和字典,all_qrcodes用于存储4个二维码存储信息,qr_positions用于存储相应data对应的二维码中心位置
all_qrcodes = []
qr_positions = {}


# MQTT连接回调函数
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("已连接到MQTT Broker")
    else:
        print("连接失败, 返回码: ", rc)


# MQTT消息回调函数
def on_message(client, userdata, msg):
    if msg.topic == view_topic and msg.payload.decode("utf-8") == "开始视觉识别":
        print("接收到'开始视觉识别'消息,等待2秒后进行识别操作...")
        time.sleep(2)
        check_books_all()
        print("标志位变true")


# MQTT发布消息回调函数
def on_publish(client, userdata, mid):
    print("消息已发布")


# 创建MQTT客户端并设置回调函数
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
# client.on_publish = on_publish

# 连接到MQTT Broker
client.connect(broker_address, broker_port, 60)
# 启动MQTT事件循环
client.loop_start()
# 启动MQTT消息监听
client.subscribe(view_topic)

# 等待连接成功
while not client.is_connected():
    pass


def check_books_all():
    # 识别次数计数器
    retry_count = 0
    while retry_count < 5:
        # 判断是否识别到全部4个二维码
        if len(all_qrcodes) == 4 and shelf_num is not None:
            # 检查中间3个二维码是否都以书架编号开头
            is_all_correct = all(code.startswith(shelf_num + "-") for code in all_qrcodes if code != r_code)
            if not is_all_correct:
                # 找出哪个二维码不是以书架编号开头
                wrong_book = next(
                    code for code in all_qrcodes if not code.startswith(shelf_num + "-") and code != r_code)
                # 根据位置关系推断出错误书籍是从左到右第几本
                sorted_qr_positions = sorted(qr_positions.items(), key=lambda x: x[1][0])
                wrong_book_index = [code for code in [item[0] for item in sorted_qr_positions] if code != r_code].index(
                    wrong_book) + 1
                # 输出错误信息
                print(f"书籍 '{wrong_book}' 在书架 {shelf_num} 上放置错误。它应该是从左到右第 {wrong_book_index} 本。")
                # 发布错误信息到MQTT主题
                client.publish(wrong_book_topic, f"{wrong_book_index}")
                client.publish(location_topic, f"{shelf_num}-{wrong_book_index}")
                client.publish(view_topic, "视觉识别完成")
            else:
                print(f"所有书籍在书架 {shelf_num} 上放置正确。")
                client.publish(wrong_book_topic, "没有错书")
                client.publish(view_topic, "视觉识别完成")
            break  # 退出循环

        else:
            print(f"未识别到全部二维码,正在重试...")
            retry_count += 1
            # 添加延迟,让相机有时间扫描二维码
            time.sleep(1)
    else:
        print(f"已经重试5次,无法识别全部4个二维码。")
        client.publish(view_topic, "视觉识别完成")
        client.publish(wrong_book_topic, "没有扫到全部二维码")


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

    # 找到以-R结尾的二维码
    r_code = next((code for code in all_qrcodes if code.endswith("-R")), None)
    if r_code:
        shelf_num = r_code[:2]
    else:
        shelf_num = None

    # 在图像上绘制识别到的二维码边界框和文本
    for qrcode in qrcodes:
        x, y, w, h = qrcode.rect
        cv2.rectangle(frame, (x, y), (x + w, y + h), (0, 255, 0), 2)
        cv2.putText(frame, qrcode.data.decode("utf-8"), (x, y - 10), cv2.FONT_HERSHEY_SIMPLEX, 0.5, (36, 255, 12), 2)

    # 在屏幕右下角打印每本书的坐标
    font = cv2.FONT_HERSHEY_SIMPLEX
    font_scale = 0.5
    font_color = (0, 0, 255)  # 红色
    font_thickness = 1
    frame_height, frame_width, _ = frame.shape
    text_x = frame_width - 300
    text_y = frame_height - 30
    sorted_qr_positions = sorted(qr_positions.items(), key=lambda x: x[1][0])
    for book_code, book_center in sorted_qr_positions:
        if book_code != r_code:
            cv2.putText(frame, f"{book_code}: ({book_center[0]}, {book_center[1]})", (text_x, text_y), font, font_scale,
                        font_color, font_thickness)
            text_y -= 30

    # 显示处理后的图像
    cv2.imshow("QR Code Reader", frame)
    # 按'Esc'退出
    key = cv2.waitKey(1) & 0xFF
    if key == 27:  # 'Esc' 键
        client.loop_stop()
        client.disconnect()
        cap.release()
        cv2.destroyAllWindows()
