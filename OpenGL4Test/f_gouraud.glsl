#version 330 core
in vec4 vColor; // 接收 Vertex Shader 輸出的顏色
out vec4 FragColor;
void main()
{
	FragColor = vColor;
}
