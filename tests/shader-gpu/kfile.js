let project = new Project('Shader-GPU');

await project.addProject(findKore());

project.addFile('sources/**');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');

project.flatten();

resolve(project);
