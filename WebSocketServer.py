
import websockets
import asyncio

cs=set()

async def talk(websocket,path):
	try:
		while 1:
			if (not websocket in cs):
				cs.add(websocket)
				msg='Welcome:'+str(websocket.remote_address)
			else:
				#msg=str(websocket.remote_address)+':'+str(await websocket.recv())
				msg=str(await websocket.recv())
			await asyncio.wait([ws.send(msg) for ws in cs])
			print(msg)
	except Exception as err:
		cs.remove(websocket)

#print('Starting...',end='\n')
start_server=websockets.serve(talk,'localhost',9766)
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
