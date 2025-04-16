import matplotlib.pyplot as plt

# Initialize a dictionary to store data for each process
data = {}

# Read and process the log file
with open('temp.txt', 'r') as file:
    for line in file:
        values = line.strip().split()
        if len(values) >= 3:
            # Extract timestamp, process_id, and queue_level
            timestamp = int(values[0].split('=')[1])
            process_id = int(values[1].split('=')[1])
            queue_level = int(values[2].split('=')[1])
            # if timestamp > 350:
            #     break
            # Skip processes 1, 2, 3
            if process_id in [1, 2, 3]:
                continue

            # Store the data points in the dictionary based on process_id
            if process_id not in data:
                data[process_id] = {'x': [], 'y': []}

            data[process_id]['x'].append(timestamp)
            data[process_id]['y'].append(queue_level)

# Plot the data
plt.figure(figsize=(10, 6))
for process_id, points in data.items():
    plt.plot(points['x'], points['y'], label=f'Process {process_id}')

# Label the axes
plt.xlabel("Time (ticks)")
plt.ylabel("Queue Level")
plt.title("MLFQ Scheduling Timeline with Queue Transitions")

# Invert y-axis and set ticks to display only integer values for queue levels 0, 1, 2, 3
plt.gca()  # Higher priority at the top
plt.yticks([0, 1, 2, 3])  # Explicitly set y-ticks to 0, 1, 2, 3

plt.gca
# Show legend
plt.legend()

# Show the plot
plt.show()
