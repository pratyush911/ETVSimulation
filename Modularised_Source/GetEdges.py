with open("/home/icy/Desktop/VR-Simulation/ETV-Simulator/Data/mesh_models/faces.txt", 'r') as f:
	line = f.readline()
	dick = {}
	edgectr = {}
	while(line):
		tr = line.split()
		tr = tr[1:]
		for i in range(3):
			a = tr[i]
			b = tr[(i+1)%3]
			if(a<b):
				if not a in dick:
					dick[a] = {b}
				else:
					dick[a].add(b)

				key = str(a)+" "+str(b)
				if key in edgectr:
					edgectr[key] = edgectr[key] + 1
				else:
					edgectr[key] = 1
			else:
				if not b in dick:
					dick[b] = {a}
				else:
					dick[b].add(a)

				key = str(b)+" "+str(a)
				if key in edgectr:
					edgectr[key] = edgectr[key] + 1
				else:
					edgectr[key] = 1
		line = f.readline()

	g = open("/home/icy/Desktop/VR-Simulation/ETV-Simulator/Data/mesh_models/edges.txt", 'w')
	for k in dick:
		s = dick[k]
		for el in s:
			g.write('e ' + k + " " + el + "\n")


	bdr = open("/home/icy/Desktop/VR-Simulation/ETV-Simulator/Data/mesh_models/boundaryPoints_ThirdVentricle.txt", 'w')
	bset = set([])
	for key in edgectr:
		if edgectr[key] > 1:
			continue
		temp = key.split()
		bset.add(int(temp[0]))
		bset.add(int(temp[1]))

	for x in bset:
		bdr.write(str(x)+"\n")
