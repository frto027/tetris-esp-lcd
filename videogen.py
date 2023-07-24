import cv2

cap = cv2.VideoCapture("Touhou - Bad Apple.mp4")

frame_count = 0

output = """uint16_t badapple[][20] = {
"""

count = 0

while True:
    ret, frame = cap.read()
    if not ret:
        break
    count += 1
    # if count > 500:
    #     break
    # if count < 400:
    #     continue

    frame = cv2.resize(frame, (20,10))
    output += "{"
    for i in range(20):
        num = 0
        for j in range(10):
            if frame[j][i][0] < 100:
                num += 1 << j
        output += f"{num},"
    frame_count += 1
    output += "},\n"
    
    # frame = cv2.resize(frame, (50,25),cv2.INTER_NEAREST_EXACT)
    # cv2.imshow('frame',frame)
    # if cv2.waitKey(1) == ord('q'):
    #     break

output += "};\n"

output += f"#define BADAPPLE_FRAME_COUNT {frame_count}\n"

with open('main/badapple.inc', 'w') as f:
    f.write(output)
print('main/badapple.inc has been generated')