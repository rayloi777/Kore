let project = new Project('Mipmap-Test');

await project.addProject(findKore());

project.addFile('sources/**');
project.addFile('deployment/**');
project.setDebugDir('deployment');

project.addIncludeDir('../../build/Kong-osx-metal');
project.addFile('../../build/Kong-osx-metal/kong.m');

project.addKongDir('shaders');

project.flatten();

resolve(project);
